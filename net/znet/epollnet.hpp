#ifndef _EPOLL_NET_H
#define _EPOLL_NET_H

#include "znet.hpp"
#include "sendbuffer.hpp"
#include "spscqueue.hpp"
#include "pool/object_pool.hpp"

#include <list>
#include <vector>
#include <thread>
#include <atomic>

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

namespace znet
{

enum SessionStatus
{
    SESSION_INIT,
    SESSION_OPEN,
    SESSION_CLOSE,
};

template< class Session>
    class Scheduler
    {
    public:
        static Scheduler< Session > & Instance()
        {
            static Scheduler< Session > scheduler;
            return scheduler;
        }

        void Start()
        {
            if(this->is_running_) return;
            this->is_running_ = true;

            // listen epoll
            this->epoll_lis_fd_ = epoll_create(10);
            assert(-1 != this->epoll_lis_fd_);

            int ret = fcntl(this->epoll_lis_fd_, F_SETFD, FD_CLOEXEC);
            assert(-1 != ret);

            // read & write epoll
            for(int i = 0; i < MAX_THREADS; i++)
            {
                this->epoll_proc_fds_[i] = epoll_create(10);
                assert(-1 != this->epoll_proc_fds_[i]);
            }
        }

        void Stop()
        {
            if(!this->is_running_) return;
            this->is_running_ = false;

            close(this->epoll_lis_fd_);
            this->epoll_lis_fd_ = 0xFFFFFF;

            for(int i = 0; i < MAX_THREADS; i++)
            {
                close(this->epoll_proc_fds_[i]);
                this->epoll_proc_fds_[i] = 0xFFFFFF;
            }
        }

        void Push(Session * session)
        {
            int32_t proc_id = session->GetFd() % this->threads_;
            int32_t epoll_proc_fd = this->epoll_proc_fds_[proc_id];
            session->SetEpollFd(epoll_proc_fd);

            // regist fd
            struct epoll_event event;
            event.events = EPOLLIN | EPOLLERR;
            event.data.ptr = session;
            epoll_ctl(epoll_proc_fd, EPOLL_CTL_ADD, session->GetFd(), &event);
        }

        void PushNullFd(int32_t fd, int32_t proc_id)
        {
            int32_t proc_fd = this->epoll_proc_fds_[proc_id];
            struct epoll_event event;
            event.events = EPOLLIN | EPOLLERR | EPOLLOUT;
            event.data.ptr = nullptr;
            epoll_ctl(proc_fd, EPOLL_CTL_ADD, fd, &event);
        }

        inline void SetThreads(int threads) { this->threads_ = threads; }
        inline int GetThreads() { return this->threads_; }
        inline int GetLisFd() { return this->epoll_lis_fd_; }
        inline int GetProcFd(int32_t proc_id) { return this->epoll_proc_fds_[proc_id]; }
    private:
        Scheduler() : is_running_(false), threads_(1)
        {}

        int32_t epoll_lis_fd_;
        int32_t epoll_proc_fds_[MAX_THREADS];
        uint8_t threads_;
        bool is_running_;
    };

////////////////////////////////////////////////////////////////////////////
template< class Session, class Factory, typename Msg = char>
    class Processor
    {
        typedef Factory * FactoryPtr;
        typedef typename Factory::MsgPackage MsgPackage;
    public:
        Processor(FactoryPtr factory = nullptr)
            : is_running_(false)
        {
            if(factory) {
                this->inner_factory_ = nullptr;
                this->factory_ = factory;
            }
            else {
                this->inner_factory_ = new Factory();
                this->factory_ = this->inner_factory_;
            }
        }

        virtual ~Processor()
        {
            if(inner_factory_)
            {
                delete this->inner_factory_;
                this->inner_factory_ = nullptr;
                this->factory_ = nullptr;
            }
        }

        void Start(const ITcpService::Properties & config, EventHandleFunc handler = nullptr)
        {
            if(this->is_running_) return;
            this->is_running_ = true;

            Scheduler< Session > & scheduler = Scheduler< Session >::Instance();

            this->event_handler_ = handler;
            this->config_ = config;

            int threads = config.threads > MAX_THREADS ? MAX_THREADS : config.threads;
            scheduler.SetThreads(threads);

            // create threads and message queue
            for(int i = 0; i < threads; i++)
            {
                // add msg queue
                this->msg_queues_.emplace_back(new MsgQueue);
                // run thread
                this->proc_threads_.emplace_back(std::thread(&Processor::Run, this, i));
            }

            // run handler message queue thread
            if(config.app_run)
                this->app_thread_ = std::thread(&Processor::AppRun, this);
        }

        void CreateNullFd(int32_t proc_id)
        {
            Scheduler< Session > & scheduler = Scheduler< Session >::Instance();
            int32_t null_fd = socket(AF_INET, SOCK_STREAM, 0);
            this->null_fds_[proc_id] = null_fd;
            scheduler.PushNullFd(null_fd, proc_id);
        }

        void Stop()
        {
            if(!this->is_running_) return;
            this->is_running_ = false;

            //Scheduler< Session > & scheduler = Scheduler< Session >::Instance();
            //close(scheduler.GetLisFd());

            // wait all thread join
            int32_t proc_id = 0;
            for(auto & thrd : this->proc_threads_)
            {
                // 发送一个空句柄
                this->CreateNullFd(proc_id++);
                thrd.join();
                // 关闭空句柄
                close(this->null_fds_[proc_id]);
            }
            this->proc_threads_.clear();

            // remove all msg queue
            for(auto & queue : this->msg_queues_)
            {
                delete queue;
            }
            this->msg_queues_.clear();

            // wait apprun thread join
            if(this->app_thread_.joinable())
            {
                this->app_thread_.join();
            }
        }

        void AppRun()
        {
            printf("Start app run thread\n");
            bool ret = false;
            while(this->is_running_)
            {
                for(auto & queue : this->msg_queues_)
                {
                    if(!queue->isEmpty())
                    {
                        MsgPackage * msg_pkg = nullptr;
                        ret = queue->try_dequeue(msg_pkg);
                        if(ret && msg_pkg)
                        {
                            msg_pkg->session->OnMessage(msg_pkg->message);
                            msg_pkg_pool_.Release(msg_pkg);
                        }
                        else
                        {
                            printf("dequeue failed.\n");
                        }
                    }
                    else
                    {
                        usleep(1);
                    }
                }

            } // end while
            printf("Stop app run thread\n");
        }

        void Run(int proc_id)
        {
            printf("Start run thread: %d\n", proc_id);
            Scheduler< Session > & scheduler = Scheduler< Session >::Instance();
            int32_t epoll_proc_fd = scheduler.GetProcFd(proc_id);
            std::list< Session *> sessions;

            auto msg_queue = this->msg_queues_[proc_id];
            struct epoll_event wait_events[MAX_WAIT_EVENT];
            int recv_len = 0;
            while(this->is_running_)
            {
                printf("wait event.\n");
                int ret = epoll_wait(epoll_proc_fd, wait_events, MAX_WAIT_EVENT, -1);// -1 wait forever
                printf("event ret: %d\n", ret);

                // wait失败
                if(ret < 0)
                {
                    printf("wait failed.");
                    continue;
                }

                // 遍历所有的事件
                for(int i = 0; i < ret; i++)
                {
                    Session * session = (Session *)wait_events[i].data.ptr;
                    // 收到空句柄就是结束的时候
                    if(nullptr == session)
                    {
                        for(auto & sess : sessions)
                        {
                            this->CloseSession(sess, epoll_proc_fd);
                        }
                        sessions.clear();

                        continue;
                    }

                    // IN事件
                    if(EPOLLIN == (wait_events[i].events & EPOLLIN))
                    {
                        // 接收到缓冲区
                        recv_len = recv(session->GetFd(), session->GetRecvBuff() + session->GetRecvBuffPos(), session->GetRecvSize(), 0);
                        // 接收错误
                        if(recv_len < 0)
                        {
                            if(errno == ECONNRESET)
                            {
                                printf("connect timeout.\n");
                            }
                            else
                            {
                                printf("read error.\n");
                            }
                            this->CloseSession(session, epoll_proc_fd);
                            sessions.remove(session);
                        }
                        // 远端正常关闭
                        else if(recv_len == 0)
                        {
                            printf("client request close.\n");
                            this->CloseSession(session, epoll_proc_fd);
                            sessions.remove(session);
                        }
                        // 正常接收
                        else
                        {
                            session->AddRecvBuffPos(recv_len);
                            size_t read_pos = 0;
                            // 断包(读取包大小)
                            int pkg_len = session->ReadPacket(session->GetRecvBuff(), session->GetRecvBuffPos());
                            while(pkg_len > 0)
                            {
                                // 如果要读取的大小足够就读取
                                if(read_pos + pkg_len <= session->GetRecvBuffPos())
                                {
                                    session->OnRecv(session->GetRecvBuff() + read_pos, pkg_len);
                                    // 启动message处理线程
                                    if(this->config_.app_run)
                                    {
                                        // 消息解码
                                        Msg * msg = session->ParseMessage(session->GetRecvBuff() + read_pos, pkg_len);
                                        MsgPackage * msg_pkg = msg_pkg_pool_.Acquire();
                                        msg_pkg->session = session;
                                        msg_pkg->message = msg;
                                        // 插入消息队列中
                                        msg_queue->enqueue(msg_pkg);
                                    }
                                    // 直接处理消息
                                    else if(this->config_.parser)
                                    {
                                        Msg * msg = session->ParseMessage(session->GetRecvBuff() + read_pos, pkg_len);
                                        session->OnMessage(msg);
                                    }

                                    // 更新已读位置
                                    read_pos += pkg_len;
                                }

                                // 判断后面是否还能继续读取
                                if(read_pos < session->GetRecvBuffPos())
                                {
                                    // 断包
                                    pkg_len = session->ReadPacket(session->GetRecvBuff() + read_pos, session->GetRecvBuffPos() - read_pos);
                                    // 如果不够就移动缓存区
                                    if(pkg_len <= 0)
                                    {
                                        memmove(session->GetRecvBuff(), session->GetRecvBuff() + read_pos, session->GetRecvBuffPos() - read_pos);
                                        session->SetRecvBuffPos(session->GetRecvBuffPos() - read_pos);
                                        *(session->GetRecvBuff() + session->GetRecvBuffPos()) = 0;
                                        break;
                                    }
                                }
                                else
                                {
                                    // 都读完了
                                    session->SetRecvBuffPos(0);
                                    break;
                                }
                            }
                        }
                    }
                    // OUT事件
                    else if(EPOLLOUT == (wait_events[i].events & EPOLLOUT))
                    {
                        // 第一次add同时会发送一个out事件用来初始
                        if(SessionStatus::SESSION_INIT == session->GetStatu())
                        {
                            session->SetStatu(SessionStatus::SESSION_OPEN);
                            sessions.emplace_back(session);
                            continue;
                        }

                        // 发送
                        session->GetSendBuffer().Pop([&](const char * data, size_t len){
                            return send(session->GetFd(), data, len, 0);
                        });

                        // 非异常失败（如无缓存区可写）则继续发送事件
                        if(errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            struct epoll_event event;
                            event.events = EPOLLIN | EPOLLOUT | EPOLLERR;
                            event.data.ptr = session;
                            epoll_ctl(session->GetEpollFd(), EPOLL_CTL_MOD, session->GetFd(), &event);
                            continue;
                        }

                        struct epoll_event event;
                        event.events = EPOLLIN | EPOLLERR;
                        event.data.ptr = session;
                        epoll_ctl(session->GetEpollFd(), EPOLL_CTL_MOD, session->GetFd(), &event);
                    }
                    // ERR事件
                    else if(EPOLLERR == (wait_events[i].events & EPOLLERR))
                    {
                        this->CloseSession(session, epoll_proc_fd);
                        sessions.remove(session);
                    }
                    // 其他事件
                    else
                    {

                    }
                } // end for
            } // end while
            printf("Stop run thread: %d\n", proc_id);
        }

        void CloseSession(Session * session, int proc_fd)
        {
            Scheduler< Session > & scheduler = Scheduler< Session >::Instance();
            epoll_ctl(proc_fd, EPOLL_CTL_DEL, session->GetFd(), nullptr);
            session->SetStatu(SessionStatus::SESSION_CLOSE);
            close(session->GetFd());
            session->OnDisconnect();
            if(this->event_handler_)
            {
                this->event_handler_(kEvtSessionRelease, session);
            }
            this->factory_->Release(session);
        }

        inline FactoryPtr GetFactory() { return this->factory_; }
    private:
        std::atomic< bool > is_running_;
        std::vector< std::thread > proc_threads_;
        std::thread app_thread_;
        // 用来关闭的空句柄
        int null_fds_[MAX_THREADS];

        typedef spscqueue<typename Factory::MsgPackage *> MsgQueue;
        std::vector< MsgQueue * > msg_queues_;
        ObjectPool< typename Factory::MsgPackage > msg_pkg_pool_;

        FactoryPtr factory_;
        FactoryPtr inner_factory_;

        EventHandleFunc event_handler_;
        ITcpService::Properties config_;
    };

////////////////////////////////////////////////////////////////////////////
template< class Session, class Factory >
    class Listener : public IListener
    {
    public:
        Listener(Processor< Session, Factory, typename Session::Message > & processor)
            : processor_(processor), address_(0), port_(0), backlogs_(10),
              listen_fd_(-1), is_running_(false)
        {}

        virtual ~Listener() {}

        virtual int Start(const ITcpService::Properties & config, EventHandleFunc handler = nullptr)
        {
            // if is running, return
            if(this->is_running_) return kRetOK;
            this->is_running_ = true;

            this->event_handler_ = handler;
            // create tcp socket
            this->listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
            if(-1 == this->listen_fd_)
            {
                printf("error socket()\n");
                return NetRet::kRetErrSocket;
            }

            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = this->address_;
            addr.sin_port = htons(this->port_);

            // set reuse socket
            if(config.reuse)
            {
                int enable = 1;
                if(setsockopt(this->listen_fd_, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0)
                {
                    printf("error setsockopt(SO_REUSEADDR)\n");
                }
            }

            // set non block
            Listener::SetSockNonBlock(this->listen_fd_);

            // bind
            if(-1 == bind(this->listen_fd_, (struct sockaddr *)&addr, sizeof(addr)))
            {
                printf("error bind()\n");
                close(this->listen_fd_);
                return NetRet::kRetErrBind;
            }

            // listen
            if(-1 == listen(this->listen_fd_, this->backlogs_))
            {
                printf("error listen()\n");
                close(this->listen_fd_);
                return NetRet::kRetErrListen;
            }

            this->run_thread_ = std::thread(&Listener::Run, this);
        }

        virtual void Stop()
        {
            // if is not running, return
            if(!this->is_running_) return;
            this->is_running_ = false;

            shutdown(this->listen_fd_, SHUT_RDWR);
            this->run_thread_.join();
        }

        /**
         * @brief Run accept thread
         */
        virtual void Run()
        {
            Scheduler< Session > & scheduler = Scheduler< Session >::Instance();
            int epoll_fd = scheduler.GetLisFd();

            // regist listen event
            struct epoll_event event;
            event.data.fd = this->listen_fd_;
            event.events = EPOLLIN;
            int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->listen_fd_, &event);
            assert(-1 != ret);

            struct sockaddr_in addr;
            struct epoll_event wait_events[MAX_WAIT_EVENT];
            int i = 0;
            int new_fd = -1;
            int addrlen = sizeof(addr);
            Session * session = nullptr;

            while(this->is_running_)
            {
                // wait event
                ret = epoll_wait(epoll_fd, wait_events, MAX_WAIT_EVENT, -1);

                // wait error
                if(ret < 0)
                {
                    printf("error epoll_wait()\n");
                    break;
                }

                if(!this->is_running_) break;

                // handler every event
                for(i = 0; i < ret; i++)
                {
                    if(this->listen_fd_ == wait_events[i].data.fd
                            && ( (EPOLLIN == wait_events[i].events) & EPOLLIN ) )
                    {
                        // accept new connection
                        new_fd = accept(this->listen_fd_, (struct sockaddr *)&addr, (socklen_t*)&addrlen);
                        if(new_fd < 0)
                        {
                            if( (errno == EAGAIN) || (errno == EWOULDBLOCK))
                                continue;
                            else
                                assert(false);
                        }

                        // set non block
                        this->SetSockNonBlock(new_fd);

                        // session with client
                        auto fact = this->processor_.GetFactory();
                        if(fact) {
                            session = fact->Create();
                        }
                        else {
                            printf("factory null\n");
                            continue;
                        }
                        if(this->event_handler_)
                        {
                            // can create own session
                            void * rt = this->event_handler_(NetEvent::kEvtSessionCreate, session);
                            if(rt != nullptr)
                                session = (Session *)session;
                        }

                        assert(session != nullptr);
                        session->SetFd(new_fd);
                        session->SetAddress(addr.sin_addr.s_addr);
                        session->SetPort(ntohs(addr.sin_port));
                        session->SetIsPassive(true);
                        session->SetStatu(SessionStatus::SESSION_INIT);

                        // epoll regist
                        scheduler.Push(session);
                        printf("new connection.\n");
                    }
                }
            }
            printf("exit listener run thread\n");
        }

        inline void SetAddress(u_long address)
        {
            this->address_ = address;
        }

        inline void SetPort(uint16_t port)
        {
            this->port_ = port;
        }

        inline void SetBacklogs(int16_t backlogs)
        {
            this->backlogs_ = backlogs;
        }

        /**
         * @brief Set socket non block
         *
         * @param sock
         */
        static void SetSockNonBlock(int32_t sock)
        {
            int opts = fcntl(sock, F_GETFL, 0);
            if(opts < 0)
            {
                printf("error fcntl() F_GETFL\n");
                return;
            }

            if(fcntl(sock, F_SETFL, opts | O_NONBLOCK) < 0)
            {
                printf("error fcntl() F_SETFL\n");
                return;
            }
        }

    private:
        Processor< Session, Factory, typename Session::Message > & processor_;
        u_long address_;
        uint16_t port_;
        uint16_t backlogs_;
        int listen_fd_;
        EventHandleFunc event_handler_;

        std::thread run_thread_;
        std::atomic< bool > is_running_;
    };

////////////////////////////////////////////////////////////////////////////
template < class Session, class Factory >
    class Connector : public IConnector
    {
    public:
        Connector(Processor< Session, Factory, typename Session::Message> & processor)
            : processor_(processor)
        {}

        virtual ~Connector() {}

        virtual int Start(EventHandleFunc handler = nullptr)
        {
            this->event_handler_ = handler;
            return Connect();
        }

        virtual void Stop()
        {
            close(this->fd_);
        }

        virtual int Connect()
        {
            // create socket
            this->fd_ = socket(AF_INET, SOCK_STREAM, 0);
            if(-1 == this->fd_)
            {
                printf("error socket()\n");
                return NetRet::kRetErrSocket;
            }
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = this->address_;
            addr.sin_port = htons(this->port_);
            int addrlen = sizeof(addr);

            // connect
            if(-1 == connect(this->fd_, (sockaddr *)&addr, addrlen))
            {
                printf("error connect()\n");
                return NetRet::kRetErrConnect;
            }

            // session with server
            session = this->processor_.GetFactory()->Create();
            if(this->event_handler_)
            {
                // can create own session
                void * rt = this->event_handler_(NetEvent::kEvtSessionCreate, session);
                if(rt != nullptr)
                    session = (Session *)session;
            }

            assert(session != nullptr);
            session->SetFd(this->fd_);
            session->SetAddress(addr.sin_addr.s_addr);
            session->SetPort(ntohs(addr.sin_port));
            session->SetIsPassive(false);
            session->SetStatu(SessionStatus::SESSION_INIT);

            // epoll regist
            Scheduler< Session >::Instance().Push(session);
        }

        inline void SetAddress(u_long address)
        {
            this->address_ = address;
        }

        inline void SetPort(uint16_t port)
        {
            this->port_ = port;
        }

        inline int GetFd(int fd)
        {
            return this->fd_;
        }

    private:
        u_long address_;
        uint16_t port_;
        int32_t fd_;
        EventHandleFunc event_handler_;
        Session * session;
        Processor< Session, Factory, typename Session::Message > & processor_;
    };

////////////////////////////////////////////////////////////////////////////
template < int RecvBufferSize = 8192,int SendBufferSize = 8192, typename Msg = char >
    class EventHandler : public IEventHandler, public IConnection
    {
    public:
        typedef Msg Message;
        EventHandler()
            : recv_buff_pos_(0)
        {}

        int AsyncSend(const char * data, size_t len)
        {
            bool ret = this->send_buff_.Push(data, len);

            // send event to fd
            struct epoll_event event;
            event.events = EPOLLIN | EPOLLOUT | EPOLLERR;
            event.data.ptr = this;
            epoll_ctl(this->epoll_fd_, EPOLL_CTL_MOD, this->fd_, &event);

            return ret ? NetRet::kRetOK : NetRet::kRetErr;
        }

        int Send(const char * data, size_t len)
        {
            // send all buffer first
            if(this->send_buff_.Peek() > 0)
                return this->AsyncSend(data, len);

            ssize_t send_len = send(this->fd_, data, len, 0);
            if(send_len > 0)
            {
                assert(send_len == len);
                this->OnSend(data, send_len);
            }
            else
            {
                if(errno == EAGAIN || errno == EWOULDBLOCK)
                    this->AsyncSend(data, len);
            }

            return send_len;
        }

        virtual void Close()
        {
            close(this->fd_);
        }

        int GetRecvSize()
        {
            return RecvBufferSize - recv_buff_pos_;
        }

        virtual void OnConnect() {}
        virtual void OnDisconnect() {}
        virtual void OnSend(const char * data, size_t len) {}
        virtual void OnRecv(const char * data, size_t len) {}
        virtual void OnMessage(Message * msg) {}

        virtual int ReadPacket(const char * data, size_t data_len)
        {
            return data_len;
        }
        virtual Message * ParseMessage(const char * data, size_t len)
        {
            return nullptr;
        }

        inline void SetFd(int fd) { this->fd_ = fd; }
        inline void SetEpollFd(int epoll_fd) { this->epoll_fd_ = epoll_fd; }
        inline void SetIsPassive(bool is_passive) { this->is_passive_ = is_passive; }
        inline void SetAddress(u_long address) { this->address_ = address; }
        inline void SetPort(uint16_t port) { this->port_ = port; }
        inline void SetStatu(SessionStatus statu) { this->statu_ = statu; }
        inline void SetRecvBuffPos(int pos) { this->recv_buff_pos_ = pos; }
        inline void AddRecvBuffPos(int size) { this->recv_buff_pos_ += size; }

        inline int GetFd() { return this->fd_; }
        inline int GetEpollFd() { return this->epoll_fd_; }
        inline SessionStatus GetStatu() { return this->statu_; }
        inline bool GetIsPassive() { return this->is_passive_; }
        inline char * GetRecvBuff() { return (char *)this->recv_buff_; }
        inline int GetRecvBuffPos() { return this->recv_buff_pos_; }
        inline SendBuffer<SendBufferSize> & GetSendBuffer() { return this->send_buff_; }

    private:
        bool is_passive_; // server true. client false;
        int fd_;
        int epoll_fd_;
        u_long address_;
        uint16_t port_;
        SessionStatus statu_;

        SendBuffer<SendBufferSize> send_buff_;
        int recv_buff_pos_;
        char recv_buff_[RecvBufferSize];
    };
////////////////////////////////////////////////////////////////////////////
template < class Session, class Factory = SessionFactory< Session > >
    class TcpService : public ITcpService
    {

    public:
        TcpService(Factory * factory = nullptr)
            : processor_(factory)
        {}

        virtual ~TcpService()
        {
            // delete listener
            while(!this->listener_list_.empty())
            {
                delete this->listener_list_.front();
                this->listener_list_.pop_front();
            }

            // delete connector
            while(!this->connector_list_.empty())
            {
                delete this->connector_list_.front();
                this->connector_list_.pop_front();
            }
        }

        virtual void Start(EventHandleFunc handler = nullptr)
        {
            this->event_handler_ = handler;
            if(this->config.backlogs <= 0) this->config.backlogs = 10;
            if(this->config.threads <= 0) this->config.threads = 1;

            // start scheduler
            auto & scheduler = Scheduler< Session >::Instance();
            scheduler.Start();

            // start processor
            this->processor_.Start(this->config, this->event_handler_);

            // start litener
            int ret = 0;
            for(auto & lis : this->listener_list_)
            {
                ret = lis->Start(this->config, this->event_handler_);
                if(this->event_handler_ != nullptr)
                    event_handler_(ret == 0 ? NetEvent::kEvtListen : NetEvent::kEvtListenFail, nullptr);
            }

            // start connector
            for(auto & conn : this->connector_list_)
            {
                ret = conn->Start(this->event_handler_);
                if(this->event_handler_ != nullptr)
                    event_handler_(ret == 0 ? NetEvent::kEvtConnect : NetEvent::kEvtConnectFail, nullptr);
            }
        }

        virtual void Stop()
        {

            // stop listener
            for(auto & lis : this->listener_list_)
            {
                lis->Stop();
            }

            // stop processor
            this->processor_.Stop();

            // stop connector
            for(auto & conn : this->connector_list_)
            {
                conn->Stop();
            }

            // stop scheduler
            Scheduler< Session >::Instance().Stop();
        }

        virtual void AddListener(const char * address, uint16_t port)
        {
            auto lis = new Listener< Session, Factory >(this->processor_);
            assert(lis != nullptr);

            lis->SetAddress(inet_addr(address));
            lis->SetPort(port);
            lis->SetBacklogs(this->config.backlogs);

            this->listener_list_.emplace_back(lis);
        }

        virtual void AddConnector(const char * address, uint16_t port)
        {
            auto conn = new Connector< Session, Factory >(this->processor_);
            assert(conn != nullptr);

            conn->SetAddress(inet_addr(address));
            conn->SetPort(port);

            this->connector_list_.emplace_back(conn);
        }

    private:
        typedef std::list< Listener< Session, Factory > * > ListenerList;
        typedef std::list< Connector< Session, Factory > * > ConnectorList;

        EventHandleFunc event_handler_;
        ListenerList listener_list_;
        ConnectorList connector_list_;
        Processor< Session, Factory, typename Session::Message > processor_;
    };

////////////////////////////////////////////////////////////////////////////

} // namespace znet

#endif // _EPOLL_NET_H
