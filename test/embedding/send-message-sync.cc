#include <lwnode-public.h>
#include <message-port.h>
#include <filesystem>
#include <future>
#include <iostream>
#include <memory>
#include <thread>

#define COUNT_OF(array) (sizeof(array) / sizeof((array)[0]))

class Info {
 public:
  Info(std::string name, std::string age, std::string gender)
      : name_(name), age_(age), gender_(gender) {}

  std::string GetName() const { return name_; }
  std::string GetAge() const { return age_; }
  std::string GetGender() const { return gender_; }

 private:
  std::string name_;
  std::string age_;
  std::string gender_;
};

int main(int argc, char* argv[]) {
  std::shared_ptr<Info> info = std::make_shared<Info>("John", "30", "male");

  lwnode::Runtime::Configuration configuration;
  configuration.OnSendMessageSync(
      [](const std::string& message, void* user_data) -> std::string {
        Info* info = static_cast<Info*>(user_data);

        if (message == "name") return info->GetName();
        if (message == "age") return info->GetAge();
        if (message == "gender") return info->GetGender();

        return "";
      },
      (void*)info.get());

  auto runtime = std::make_shared<lwnode::Runtime>(std::move(configuration));

  std::promise<void> promise;
  std::future<void> init_future = promise.get_future();
  const char* script = "test/embedding/test-10-send-message-sync-basic.js";
  std::string path = (std::filesystem::current_path() / script).string();
  char* args[] = {const_cast<char*>(""), const_cast<char*>(path.c_str())};

  std::thread worker = std::thread(
      [&](std::promise<void>&& promise) mutable {
        // FIXME: Fix Runtime::Init() call to ensure environment initialization
        // before running the loop, Runtime::Run(). This workaround passes a
        // promise directly to know when that is.
        runtime->Start(COUNT_OF(args), args, std::move(promise));
      },
      std::move(promise));

  init_future.wait();

  int count1 = 0;
  auto port2 = runtime->GetPort();
  port2->OnMessage([&](const MessageEvent* event) {
    std::cout << event->data() << std::endl;
    count1++;
  });
  port2->PostMessage(MessageEvent::New("ping"));

  worker.join();
  return 0;
}
