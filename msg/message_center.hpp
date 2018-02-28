#ifndef _MESSAGE_CENTER_HPP
#define _MESSAGE_CENTER_HPP

#include "protobuf_processor.hpp"
#include "event_processor.hpp"
#include "timer_processor.hpp"
#include "module_message_processor.hpp"

#include <module_mgr.hpp>
#include <config_manager.h>
#include <unordered_map>
#include <queue>
#include <vector>
#include <thread>

class MessageCenter : public IMessageCenter
{
public:
    typedef std::shared_ptr< ModuleProcessor > ModuleProcPtr;

    static MessageCenter & Instance() {
        static MessageCenter instance;
        return instance;
    }

    bool Initialize() {
        works_count_ = atoi(GConfig.GetConfigParam("msg.threads", "1").c_str());
        running_ = false;
        processors_count_ = 0;
        no_message_count_ = 0;

        auto modules = GModules.GetAllModule();

        ProtobufProcessor::Instance().SetMessageCenter(this);
        EventProcessor::Instance().SetMessageCenter(this);
        TimerProcessor::Instance().SetMessageCenter(this);

        for(auto module : modules) {
            ModuleProcPtr proc(new ModuleProcessor);
            processors_map_[module.first] = proc;
            processors_que_.push(proc);
            processors_count_++;
        }
        return true;
    }

    bool Start() {
        running_ = true;
        // running work thread
        for(int i = 0; i < works_count_; i++) {
            threads_.emplace_back(std::thread(&MessageCenter::Run, this));
        }

        return true;
    }

    void Stop() {
        running_ = false;
        for(auto & thread : threads_) {
            thread.join();
        }
        threads_.clear();

        processors_map_.clear();
        while(!processors_que_.empty()) {
            processors_que_.front()->Stop();
            processors_que_.pop();
        }
    }

    virtual void InsertEventMessage(const std::string & module_name, EventCB & cb, proto::EventBuff * buff) {
        auto find = processors_map_.find(module_name);

        if(find != processors_map_.end()) {
            find->second->InsertEventMessage(cb, buff);
        }
        else {
            if(buff != nullptr) { delete buff; }
            logger_warn("not found event module: {}", module_name);
        }
    }

    virtual void InsertProtobufMessage(const std::string & module_name, ISender * sender, proto::Protobuf * buff) {
        auto find = processors_map_.find(module_name);

        if(find != processors_map_.end()) {
            find->second->InsertProtobufMessage(sender, buff);
        }
        else {
            if(buff != nullptr) { delete buff; }
            logger_warn("not found event module: {}", module_name);
        }
    }

    ITimerProc * FindTimerProc()
    {
        auto id = std::this_thread::get_id();

        for(auto & proc : processors_map_) {
            if(proc.second->GetCurrentLoopThreadId() == id) {
                return proc.second.get();
            }
        }
        return nullptr;
    }

private:
    void Run() {
        while(running_) {
            std::unique_lock< std::mutex > lck(lock_);
            auto proc = processors_que_.front();
            processors_que_.pop();

            lck.unlock();
            if(!proc->Loop()) {
                no_message_count_++;
            }
            else {
                no_message_count_ = 0;
            }

            lck.lock();
            processors_que_.push(proc);
            lck.unlock();

            if(no_message_count_ >= processors_count_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                no_message_count_--;
            }
        }
    }

private:
    MessageCenter() = default;
    ~MessageCenter() = default;

    uint8_t works_count_;
    uint16_t processors_count_;
    std::atomic< bool > running_;
    std::atomic< int > no_message_count_;
    std::thread timer_thread_;
    std::vector< std::thread > threads_;
    std::unordered_map< std::string, ModuleProcPtr > processors_map_;
    std::queue< ModuleProcPtr > processors_que_;
    std::mutex lock_;
};

#define GMessage MessageCenter::Instance()
#endif // _MESSAGE_CENTER_HPP
