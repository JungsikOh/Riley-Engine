#pragma once

namespace Riley
{
class Log
{
   public:
   static void Initialize();

   static std::shared_ptr<spdlog::logger>& GetCoreLogger()
   {
      return s_CoreLogger;
   }
   static std::shared_ptr<spdlog::logger>& GetClientLogger()
   {
      return s_ClientLogger;
   }

   private:
   static std::shared_ptr<spdlog::logger> s_CoreLogger;
   static std::shared_ptr<spdlog::logger> s_ClientLogger;
};
} // namespace Riley

#define RI_CORE_TRACE(...) ::Riley::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define RI_CORE_INFO(...) ::Riley::Log::GetCoreLogger()->info(__VA_ARGS__)
#define RI_CORE_WARN(...) ::Riley::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define RI_CORE_ERROR(...) ::Riley::Log::GetCoreLogger()->error(__VA_ARGS__)
#define RI_CORE_CRITICAL(...) ::Riley::Log::GetCoreLogger()->critical(__VA_ARGS__)

#define RI_TRACE(...) ::Riley::Log::GetClientLogger()->trace(__VA_ARGS__)
#define RI_INFO(...) ::Riley::Log::GetClientLogger()->info(__VA_ARGS__)
#define RI_WARN(...) ::Riley::Log::GetClientLogger()->warn(__VA_ARGS__)
#define RI_ERROR(...) ::Riley::Log::GetClientLogger()->error(__VA_ARGS__)
#define RI_CRITICAL(...) ::Riley::Log::GetClientLogger()->critical(__VA_ARGS__)
