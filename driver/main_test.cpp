#include <request_srv/request.hpp>
#include <iostream>

int main(){

  try{
        using namespace RQH;

        RequestCls obj("url","at","rt","ci","cs","/home/nbaskey/Pictures/test_folder");

        // RequestCls obj("url","at","rt","ci","cs","/home/nbaskey/no-dir");
        std::vector<std::string> file_list_vec = obj.checkIfDirectoryHasJPG();
        if(file_list_vec.size() == 0){
          std::cout << "No image files present\n";

          return 1;
        }

        std::string zip_file_name = obj.zipAndRemove(file_list_vec);
        // std::cout << zip_file_name << "\n";
  }catch(std::runtime_error& e){
    std::cout << "ERROR occurred : " << e.what() << "\n";
  }
  return 0;
}
