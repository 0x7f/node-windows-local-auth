#define SECURITY_WIN32 1

#include <node.h>
#include <nan.h>
#include <v8.h>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace {

#ifdef _WIN32
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
#endif

    class CheckUserPassword : public Nan::AsyncWorker {
    public:
        CheckUserPassword(Nan::Callback *callback, const std::string& domain, const std::string& user, const std::string& password)
            : Nan::AsyncWorker(callback), domain(domain), user(user), password(password), success(false), administrator(false) { }

        ~CheckUserPassword() { }

        virtual void Execute() {
#ifdef _WIN32
            //std::wstring swDomain = std::wstring(domain.begin(), domain.end());
            //std::wstring swUser = std::wstring(user.begin(), user.end());
            //std::wstring swPassword = std::wstring(password.begin(), password.end());
            std::wstring swDomain = ConvertUtf8ToWide(domain);
            std::wstring swUser = ConvertUtf8ToWide(user);
            std::wstring swPassword = ConvertUtf8ToWide(password);
            LPCTSTR lpDomain = swDomain.c_str();
            LPCTSTR lpUser = swUser.c_str();
            LPCTSTR lpPass = swPassword.c_str();

            HANDLE hdl;
            if (!LogonUser(lpUser, lpDomain, lpPass, LOGON32_LOGON_NETWORK, LOGON32_PROVIDER_DEFAULT, &hdl)) {
                DWORD err = ::GetLastError();
                if (err != ERROR_LOGON_FAILURE && err != ERROR_ACCOUNT_EXPIRED) {
                    SetErrorMessage(GetLastErrorAsString().c_str());
                }
                return;
            }

            if (!hdl) {
                SetErrorMessage("LogonUser returned invalid token");
                return;
            }

            SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
            DWORD subAuth0 = SECURITY_BUILTIN_DOMAIN_RID;
            DWORD subAuth1 = DOMAIN_ALIAS_RID_ADMINS;
            PSID AdminGroup;
            if (!AllocateAndInitializeSid(&NtAuthority, 2, subAuth0, subAuth1, 0, 0, 0, 0, 0, 0, &AdminGroup)) {
                SetErrorMessage(GetLastErrorAsString().c_str());
                return;
            }

            BOOL isAdmin;
            BOOL rc = CheckTokenMembership(hdl, AdminGroup, &isAdmin);
            FreeSid(AdminGroup);
            if (!rc) {
                SetErrorMessage(GetLastErrorAsString().c_str());
                return;
            }

            if (!CloseHandle(hdl)) {
                SetErrorMessage(GetLastErrorAsString().c_str());
                return;
            }

            success = true;
            administrator = isAdmin;
#else
            SetErrorMessage("Your operating system is not supported.");
#endif
        }

        void HandleErrorCallback() {
            Nan::HandleScope scope;
            v8::Local <v8::Value> argv[] = { Nan::Error(ErrorMessage()) };
            callback->Call(Nan::GetCurrentContext()->Global(), 1, argv, async_resource);
        }

        void HandleOKCallback() {
            Nan::HandleScope scope;
            v8::Local <v8::Value> returnValueSuccess = Nan::New<v8::Boolean>(success);
            v8::Local <v8::Value> returnValueAdmin = Nan::New<v8::Boolean>(administrator);
            v8::Local <v8::Value> argv[] = { Nan::Null(), returnValueSuccess, returnValueAdmin };
            callback->Call(Nan::GetCurrentContext()->Global(), 3, argv, async_resource);
        }

    private:
        std::string domain;
        std::string user;
        std::string password;
        bool success;
        bool administrator;
        std::wstring ConvertUtf8ToWide(const std::string& str) {
            int count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0);
            std::wstring wstr(count, 0);
            MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &wstr[0], count);
            return wstr;
        }
    };

    NAN_METHOD(checkUserPassword) {
        Nan::Utf8String domain(info[0]);
        Nan::Utf8String user(info[1]);
        Nan::Utf8String password(info[2]);
        Nan::Callback *callback = new Nan::Callback(info[3].As<v8::Function>());
        Nan::AsyncQueueWorker(new CheckUserPassword(callback, *domain, *user, *password));
    }

    NAN_MODULE_INIT(init) {
        Nan::HandleScope scope;
        Nan::SetMethod(target, "checkUserPassword", checkUserPassword);
    }

    NODE_MODULE(windows_local_auth, init)

} // anonymous namespace
