#pragma once
#include <string>
#include <fstream>
#include <sys/stat.h>

class CredentialsFile
{
public:
    explicit CredentialsFile(const std::string &path = "credentials.txt")
        : path_(path) {}

    bool exists() const
    {
        struct stat st{};
        return (stat(path_.c_str(), &st) == 0 && st.st_size > 0);
    }

    std::string load() const
    {
        std::ifstream file(path_);
        std::string username;
        std::getline(file, username);
        return username;
    }

    bool save(const std::string &username) const
    {
        std::ofstream file(path_, std::ios::trunc);
        if (!file.is_open())
            return false;
        file << username << "\n";
        return file.good();
    }

private:
    std::string path_;
};

