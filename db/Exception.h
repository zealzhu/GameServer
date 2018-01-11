#include <exception>
namespace GameDB
{

enum ErrorCode
{
    kNoAvailiableConnection,
};

class Exception : std::exception
{
public:
    Exception(ErrorCode code) : error_code_(code) {}

    inline const char * ErrorMessage() const
    {
        switch(this->error_code_)
        {
            case ErrorCode::kNoAvailiableConnection:
            return "无可用连接";
        default:
            return "未知错误";
        }
    }

    inline ErrorCode GetCode() const { return this->error_code_; }

private:
    ErrorCode error_code_;
};
}
