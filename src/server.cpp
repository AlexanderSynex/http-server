#include <algorithm>
#include <arpa/inet.h>
#include <array>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <shared_mutex>
#include <stop_token>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

using addr_t = sockaddr_in;

struct SocketFile
{
  SocketFile (std::filesystem::path p) : file (p)
  {
    if (std::filesystem::exists (p))
      unlink (p.c_str ());
  };

  virtual ~SocketFile ()
  {
    std::cout << "unlinking: " << file.c_str () << std::endl;
    unlink (file.c_str ());
  }

  std::filesystem::path file;
};

struct Connection
{
  void
  close ()
  {
    ::close (socket);
    socket = -1;
  }

  bool
  is_free () const
  {
    return socket == -1;
  }
  int socket = -1;
};

template <typename Type> using ptr = std::unique_ptr<Type>;

int
main ()
{
  std::stop_source ssource;
  std::shared_mutex m;
  constexpr std::size_t thread_limit = 1;
  auto thread_pool = std::array<ptr<std::jthread>, thread_limit>{};
  auto connections = std::unordered_map<std::thread::id, Connection>{};
  for (auto i = 0; i < thread_limit; ++i)
    {
      thread_pool[i] = std::make_unique<std::jthread> (
          [&cons = connections, &m, id = i] (std::stop_token stoken)
            {
              using namespace std::chrono_literals;
              {
                std::lock_guard l (m);
                std::cout << "Starting handler " << id
                          << " at: " << std::this_thread::get_id ()
                          << std::endl;
              }

              auto &connection = cons[std::this_thread::get_id ()];
              for (auto buffer = std::array<char, 1024>{ 0 };
                   not stoken.stop_requested ();)
                {
                  {
                    std::lock_guard l (m);
                    std::cout << id << ": go to sleep\n";
                  }
                  std::this_thread::sleep_for (100ms);

                  if (connection.socket == -1)
                    continue;

                  for (auto connectionValid = true; connectionValid;)
                    {
                      auto status = recv (connection.socket, buffer.data (),
                                          buffer.max_size (), 0);
                      if (status == 0)
                        {
                          connectionValid = false;
                          connection.close ();
                          connection.close ();
                          continue;
                        }

                      auto message
                          = std::string_view{ buffer.data (), buffer.size () };

                      auto page = std::string{ 
                        "HTTP/1.1 200 OK\n"
                        "Content-Type: text/html\n\n"
                        "<!DOCTYPE html>\n"
                        "<html>\n"
                        "<h1>Its a server!</h1>\n"
                        "</html>\n" };
                      send (connection.socket, page.data (), page.size (), 0);
                      connectionValid = false;
                      {
                        std::lock_guard l (m);
                        std::cout << id << ": " << message << std::endl;
                      }
                    }
                  connection.close ();
                };
            },
          ssource.get_token ());
    }

  auto sock = socket (AF_INET, SOCK_STREAM, 0);
  sockaddr_in peer, addr = { AF_INET, htons (9000), inet_addr ("127.0.0.1") };

  if (bind (sock, reinterpret_cast<const struct sockaddr *> (&addr),
            sizeof (addr))
      == -1)
    {
      perror ("Can not bind to address");
      return 1;
    }

  if (listen (sock, 20) == -1)
    {
      perror ("Can not start listening for 20 connection");
      return 1;
    }

  for (;;)
    {
      {
        std::lock_guard l (m);
        std::cout << "Waiting for connection" << std::endl;
      }
      socklen_t len = sizeof (peer);
      auto connection
          = accept (sock, reinterpret_cast<struct sockaddr *> (&peer), &len);
      if (connection == -1)
        {
          using namespace std::chrono_literals;
          std::cerr << "Skipping. Go to sleep for 1 second" << std::endl;
          std::this_thread::sleep_for (1s);
          continue;
        }

      if (auto it = std::find_if (
              connections.begin (), connections.end (),
              [] (decltype (connections)::const_reference ref) -> bool
                { return ref.second.is_free (); });
          it == connections.end ())
        {
          std::cerr << "Connection refused. No free handlers" << std::endl;
          close (connection);
          continue;
        }
      else
        {
          std::lock_guard l (m);
          it->second.socket = connection;
        }
    }

  close (sock);
  return 0;
}
