#include <request_srv/request.hpp>
#include <iostream>

int main(){

  try{
        using namespace RQH;

        RequestCls obj(
          "https://www.googleapis.com/upload/drive/v3/files?uploadType=resumable",
          "ya29.a0AVvZVso3HX9CU8fDP0q2n1rJZNpkDo_k6xFzekF1K3MbWOg4tgqzOqVSrZBCSbb6qDbtmVpmZfu4rRFp-JRPrl_-cG-S007mXEKXgo98ZlqgpWv0JVv7Dm3DoA-xK1nwBMNcNg_4fQxKhEdIr_ZKwb3Y7kPiEFIUaCgYKAdkSAQASFQGbdwaIGGm3C3OUrl_LnYiXir-caw0167",
          "1//04XPHBxX1XljjCgYIARAAGAQSNwF-L9IrxH-c4XgiM-bqC6UTczeuuxZfyQSZ7Hk3Hj22-DOWnCg_oR9vL_j_lOszu0Wujy4yL98",
          "854009361838-jbp08qs877jefp4ecni65ft79dt7raq7.apps.googleusercontent.com",
          "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",
          "/home/nbaskey/Pictures/test_folder"
        );

        // RequestCls obj("url","at","rt","ci","cs","/home/nbaskey/no-dir");
        std::vector<std::string> file_list_vec = obj.checkIfDirectoryHasJPG();
        if(file_list_vec.size() == 0){
          std::cout << "No image files present\n";

          return 1;
        }

        std::string zip_file_name = obj.zipAndRemove(file_list_vec);
        std::cout << zip_file_name << "\n";

        std::string location_header_value = obj.get_location_header_value(zip_file_name);
        if(location_header_value.empty()){
          std::cout << "Location header value got is EMPTY....\n";

          return 1;
        }

        std::string returned_response_id = obj.upload_file(location_header_value,zip_file_name);

        std::cout << "Uploaded successfully the id we got is : " << returned_response_id << "\n";



  }catch(std::runtime_error& e){
    std::cout << "ERROR occurred : " << e.what() << "\n";
  }
  return 0;
}
