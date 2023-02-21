#ifndef REQUEST_H

#define REQUEST_H
// #include <cpr/cpr.h> // Throws ERROR
// #include <nlohmann/json.hpp> // Thows ERROR

// #include "../../../external/json/include/nlohmann/json.hpp" // Okay


#include <string>
#include <filesystem>
#include <vector>

namespace RQH{
    class RequestCls{
        private:
            std::string _url;
            std::string _access_token;
            std::string _refresh_token;
            std::string _client_id;
            std::string _client_secret;
            std::filesystem::path _directory_path;

            void renew_access_token();

        public:

            RequestCls(
                std::string url,
                std::string access_token,
                std::string refresh_token,
                std::string client_id,
                std::string client_secret,
                std::filesystem::path directory_path
            ):  _url(url),_access_token(access_token),
                _refresh_token(refresh_token),_client_id(client_id),
                _client_secret(client_secret),_directory_path(directory_path){

              if(!std::filesystem::exists(_directory_path) && std::filesystem::is_directory(_directory_path)){
                throw std::runtime_error("Either the provided path does not exists or the path provided is invalid");
              }

            }

      std::string get_location_header_value(std::string);
      std::string upload_file(std::string,std::string);

      std::vector<std::string> checkIfDirectoryHasJPG();
      std::string zipAndRemove(std::vector<std::string>);
    };
}

#endif
