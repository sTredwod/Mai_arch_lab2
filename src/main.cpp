#include <userver/clients/dns/component.hpp>
#include <userver/clients/http/component_list.hpp>
#include <userver/components/component_list.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/congestion_control/component.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/utils/daemon_run.hpp>

#include "handlers/auth_login.hpp"
#include "handlers/books_create.hpp"
#include "handlers/books_get.hpp"
#include "handlers/loans_create.hpp"
#include "handlers/loans_get.hpp"
#include "handlers/loans_return.hpp"
#include "handlers/users_create.hpp"
#include "handlers/users_get.hpp"
#include "middlewares/auth_middleware.hpp"
#include "storage/library_storage_component.hpp"

int main(int argc, char* argv[]) {
    auto component_list =
        userver::components::MinimalServerComponentList()
            .Append<userver::server::handlers::Ping>()
            .Append<userver::components::TestsuiteSupport>()
            .AppendComponentList(userver::clients::http::ComponentList())
            .Append<userver::clients::dns::Component>()
            .Append<userver::server::handlers::TestsControl>()
            .Append<userver::congestion_control::Component>()
            .Append<library::LibraryStorageComponent>()
            .Append<library::AuthMiddlewareFactory>()
            .Append<library::UsersCreateHandler>()
            .Append<library::UsersGetHandler>()
            .Append<library::AuthLoginHandler>()
            .Append<library::BooksCreateHandler>()
            .Append<library::BooksGetHandler>()
            .Append<library::LoansCreateHandler>()
            .Append<library::LoansGetHandler>()
            .Append<library::LoansReturnHandler>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
