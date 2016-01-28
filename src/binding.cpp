#define SECURITY_WIN32 1

#include <node.h>
#include <nan.h>
#include <v8.h>

#include <Windows.h>

namespace {

    std::string GetLastErrorAsString() {
        DWORD errorMessageID = ::GetLastError();
        if (errorMessageID == 0)
            return std::string();

        DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
        DWORD languageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
        LPSTR messageBuffer = NULL;
        size_t size = FormatMessageA(flags, NULL, errorMessageID, languageId, (LPSTR)&messageBuffer, 0, NULL);

        std::string message(messageBuffer, size);
        LocalFree(messageBuffer);
        return message;
    }

    class CheckUserPassword : public Nan::AsyncWorker {
    public:
        CheckUserPassword(Nan::Callback *callback, const std::string& domain, const std::string& user, const std::string& password)
            : Nan::AsyncWorker(callback), domain(domain), user(user), password(password), success(false) { }

        ~CheckUserPassword() { }

        virtual void Execute() {

            std::wstring swDomain = std::wstring(domain.begin(), domain.end());
            std::wstring swUser = std::wstring(user.begin(), user.end());
            std::wstring swPassword = std::wstring(password.begin(), password.end());

            HANDLE hdl;
            if (LogonUser(swUser.c_str(), swDomain.c_str(), swPassword.c_str(), LOGON32_LOGON_NETWORK, LOGON32_PROVIDER_DEFAULT, &hdl)) {
                success = true;
                return;
            }

            switch(::GetLastError()) {
                // expected error cases
                case ERROR_LOGON_FAILURE:
                case ERROR_ACCOUNT_EXPIRED:
                    success = false;
                    break;
                // unexpected error cases
                default:
                    SetErrorMessage(GetLastErrorAsString().c_str());
                    success = false;
                    break;
            }
        }

        void HandleErrorCallback() {
            Nan::HandleScope scope;
            v8::Local <v8::Value> argv[] = { Nan::Error(ErrorMessage()) };
            callback->Call(1, argv);
        }

        void HandleOKCallback() {
            Nan::HandleScope scope;
            v8::Local <v8::Value> returnValue = Nan::New<v8::Boolean>(success);
            v8::Local <v8::Value> argv[] = { Nan::Null(), returnValue };
            callback->Call(2, argv);
        }

    private:
        std::string domain;
        std::string user;
        std::string password;
        bool success;

    };

    NAN_METHOD(checkUserPassword) {
        Nan::Utf8String domain(info[0]->ToString());
        Nan::Utf8String user(info[1]->ToString());
        Nan::Utf8String password(info[2]->ToString());
        Nan::Callback *callback = new Nan::Callback(info[3].As<v8::Function>());
        Nan::AsyncQueueWorker(new CheckUserPassword(callback, *domain, *user, *password));
    }

    NAN_MODULE_INIT(init) {
        Nan::HandleScope scope;
        Nan::SetMethod(target, "checkUserPassword", checkUserPassword);
    }

    NODE_MODULE(windows_local_auth, init)

} // anonymous namespace
