#include <request_srv/request.hpp>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>
#include <unistd.h>

using namespace nlohmann::literals;

std::string RQH::RequestCls::upload_file(std::string zip_file_to_upload)
{
  if (std::filesystem::is_regular_file(_directory_path.string() + "/" + zip_file_to_upload))
  {

    // POST https://www.googleapis.com/upload/drive/v3/files?uploadType=resumable
    bool flag = true;
    std::string location_header_value = "";

    while (flag)
    {
      std::string authorization_key = "Bearer " + _access_token;

      cpr::Response r = cpr::Post(
          cpr::Url{_url},
          cpr::Header{{"Authorization", authorization_key}});

      if (r.status_code == 200)
      {
        location_header_value = r.header["Location"];

        // If the session initiation request succeeds, the response includes a 200 OK HTTP status code.
        // In addition, the response includes a Location header that specifies the resumable session URI.
        // Use the resumable session URI to upload the file data and query the upload status.
        // A resumable session URI expires after one week

        flag = false;
      }
      else if (r.status_code == 401)
      {
        // {
        //     "error": {
        //         "errors": [
        //             {
        //                 "domain": "global",
        //                 "reason": "authError",
        //                 "message": "Invalid Credentials",
        //                 "locationType": "header",
        //                 "location": "Authorization"
        //             }
        //         ],
        //         "code": 401,
        //         "message": "Invalid Credentials"
        //     }
        // }

        nlohmann::json response_body = nlohmann::json::parse(r.text);

        if (response_body["error"]["message"] == "Invalid Credentials")
        {
          // access token is expired

          renew_access_token(); // access token is renewed here

        }
      }
    }

    return location_header_value;
  }
  else
  {
    throw std::runtime_error("zip file path provided does not exists");
  }
}

void RQH::RequestCls::renew_access_token()
{

  nlohmann::json post_request_body;

  post_request_body["client_secret"] = _client_secret;
  post_request_body["grant_type"] = "refresh_token";
  post_request_body["refresh_token"] = _refresh_token;
  post_request_body["client_id"] = _client_id;

  cpr::Response r = cpr::Post(
      cpr::Url{"https://www.googleapis.com/oauth2/v4/token"},
      cpr::Body{post_request_body.dump()},
      cpr::Header{{"Content-Type", "application/json"}});

  if (r.status_code == 200)
  {
    nlohmann::json response_body = nlohmann::json::parse(r.text);

    _access_token = response_body["access_token"];
  }
  else
  {
    // {
    //   "error": "invalid_grant",
    //   "error_description": "Bad Request"
    // }

    // refresh token provided is wrong

    throw std::runtime_error("Refresh token provided is wrong");
  }
}

std::vector<std::string> RQH::RequestCls::checkIfDirectoryHasJPG()
{

  std::vector<std::string> extension_vec{".jpg", ".jpeg", ".png"};
  std::vector<std::string> file_list_vec;
  for (auto &p : std::filesystem::recursive_directory_iterator(_directory_path))
  {
    if (std::find(extension_vec.begin(), extension_vec.end(), p.path().extension()) != extension_vec.end())
    {
      // std::cout << p.path().string() << "\n";

      file_list_vec.emplace_back(p.path().string());
    }
    else
    {
      // std::cout << "E- > " <<p.path().string() << "\n";
      if (std::filesystem::is_regular_file(p))
      {
        // std::cout << p.path().string() << "\n";
        unlink(p.path().string().c_str()); // Delete the files other than .jpg, jpeg, .png
      }
    }
  }

  return file_list_vec;
}

std::string RQH::RequestCls::zipAndRemove(std::vector<std::string> file_list_vec)
{

  std::time_t now_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  std::string zip_parameters = "";

  for (const auto &p : file_list_vec)
  {
    zip_parameters += p + " ";
  }

  std::string time_str = std::string(std::ctime(&now_time));
  std::string to_upload_new_name_path = _directory_path.string() + "/to_upload_" + time_str.erase(time_str.length() - 1);

  std::string zip_cmd = "zip '" + to_upload_new_name_path + ".zip' " + zip_parameters + " > /dev/null 2>&1";

  // std::cout << zip_cmd << "\n";

  // Unix zip command will update existing archive instead of creating a new one
  // https://stackoverflow.com/questions/38666626/unix-zip-command-is-updating-existing-archive-instead-of-creating-a-new-one
  if (system(zip_cmd.c_str()) == -1)
  {
    throw std::runtime_error("'zip' command may not be present or the command executed is not okay..");
  }

  // zip has been made successfully
  return "'" + to_upload_new_name_path + ".zip'";
}
