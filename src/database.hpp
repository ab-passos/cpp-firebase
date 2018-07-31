
#pragma once
#include <stdarg.h>
#include <vector>
#include <memory>
#include "firebase/app.h"
#include "firebase/auth.h"
#include "firebase/database.h"
#include "firebase/future.h"
#include "firebase/util.h"

struct Device {
  std::string device_name_;
  int version_;
};

struct FullDevices {
  std::string device_name_;
  int version_;
  std::string friendly_name_;
  bool has_update_;
  int next_version;
}

using Devices = std::vector<Device>;

class DatabaseConnection {
public:
  explicit DatabaseConnection(const std::string& database_name)
  : database_name_{database_name}
  , app_{::firebase::App::Create()}
  , database(nullptr)
  , auth(nullptr)
  {
    void* initialize_targets[] = {&auth, &database};

    const firebase::ModuleInitializer::InitializerFn initializers[] = {
        [](::firebase::App* app, void* data) {
          LogMessage("Attempt to initialize Firebase Auth.");
          void** targets = reinterpret_cast<void**>(data);
          ::firebase::InitResult result;
          *reinterpret_cast<::firebase::auth::Auth**>(targets[0]) =
              ::firebase::auth::Auth::GetAuth(app, &result);
          return result;
        },
        [](::firebase::App* app, void* data) {
          LogMessage("Attempt to initialize Firebase Database.");
          void** targets = reinterpret_cast<void**>(data);
          ::firebase::InitResult result;
          *reinterpret_cast<::firebase::database::Database**>(targets[1]) =
              ::firebase::database::Database::GetInstance(app, &result);
          return result;
        }};

    ::firebase::ModuleInitializer initializer;
    initializer.Initialize(app_.get(), initialize_targets, initializers,
                           sizeof(initializers) / sizeof(initializers[0]));
    LogMessage("Successfully initialized Firebase Auth and Firebase Database.");
    database->set_persistence_enabled(true);
  }

  void SignOn() {
    firebase::Future<firebase::auth::User*> sign_in_future =
        auth->SignInAnonymously();
    WaitForCompletion(sign_in_future, "SignInAnonymously");
    if (sign_in_future.error() == firebase::auth::kAuthErrorNone) {
      LogMessage("Auth: Signed in anonymously.");
    } else {
      LogMessage("ERROR: Could not sign in anonymously. Error %d: %s",
                 sign_in_future.error(), sign_in_future.error_message());
      LogMessage(
          "  Ensure your application has the Anonymous sign-in provider "
          "enabled in Firebase Console.");
      LogMessage(
          "  Attempting to connect to the database anyway. This may fail "
          "depending on the security settings.");
    }
  }

  void AddDevicesToDatabase()

  void RegisterDevices(const Devices& devices) {
    devices_ = devices;
  }

  void GetDeviceDetails(const Devices& devices) {
    for (auto const& device: devices) {

    }
  }

private:
  const std::string& database_name_;
  std::unique_ptr<::firebase::App> app_;
  std::unique_ptr<::firebase::database::Database> database;
  std::unique_ptr<::firebase::auth::Auth> auth;
  Devices devices_;

  // Wait for a Future to be completed. If the Future returns an error, it will
  // be logged.
  void WaitForCompletion(const firebase::FutureBase& future, const char* name) {
    while (future.status() == firebase::kFutureStatusPending) {
      ProcessEvents(100);
    }
    if (future.status() != firebase::kFutureStatusComplete) {
      LogMessage("ERROR: %s returned an invalid result.", name);
    } else if (future.error() != 0) {
      LogMessage("ERROR: %s returned error %d: %s", name, future.error(),
                 future.error_message());
    }
  }

};
