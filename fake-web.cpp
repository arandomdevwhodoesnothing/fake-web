#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

const std::string SITES_DIR = "./fake-web-sites";

void ensureDir() {
    fs::create_directories(SITES_DIR);
}

std::string sitePath(const std::string& address) {
    return SITES_DIR + "/" + address;
}

bool isValidAddress(const std::string& addr) {
    // Must have at least one dot, and something before and after it
    auto dot = addr.rfind('.');
    if (dot == std::string::npos || dot == 0 || dot == addr.size() - 1)
        return false;
    return true;
}

void cmdCreate(const std::string& address) {
    if (!isValidAddress(address)) {
        std::cout << "Invalid address. Use format: filename.domain (e.g. hello.com)\n";
        return;
    }
    std::string path = sitePath(address);
    if (fs::exists(path)) {
        std::cout << "Site '" << address << "' already exists.\n";
        return;
    }
    std::ofstream f(path);
    if (!f) {
        std::cout << "Failed to create site.\n";
        return;
    }
    f.close();
    std::cout << "Created '" << address << "'. Use 'edit " << address << "' to add content.\n";
}

void cmdEdit(const std::string& address) {
    std::string path = sitePath(address);
    if (!fs::exists(path)) {
        std::cout << "Site '" << address << "' not found. Create it first with: create " << address << "\n";
        return;
    }
    std::cout << "Enter content for " << address << " (type END on a new line to finish):\n";
    std::string content, line;
    while (std::getline(std::cin, line)) {
        if (line == "END") break;
        content += line + "\n";
    }
    std::ofstream f(path);
    f << content;
    std::cout << "Saved content to '" << address << "'.\n";
}

void cmdVisit(const std::string& address) {
    std::string path = sitePath(address);
    if (!fs::exists(path)) {
        std::cout << "404 Not Found: '" << address << "' does not exist.\n";
        return;
    }
    std::ifstream f(path);
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║  fake-web://  " << address;
    int pad = 35 - (int)address.size();
    for (int i = 0; i < pad; i++) std::cout << ' ';
    std::cout << "║\n";
    std::cout << "╠══════════════════════════════════════════════════╣\n";
    if (content.empty()) {
        std::cout << "║  (empty page)                                    ║\n";
    } else {
        std::istringstream ss(content);
        std::string line;
        while (std::getline(ss, line)) {
            // print each line with padding
            std::cout << "║  ";
            if (line.size() > 47) {
                std::cout << line.substr(0, 47);
            } else {
                std::cout << line;
                for (int i = 0; i < (int)(47 - line.size()); i++) std::cout << ' ';
            }
            std::cout << "║\n";
        }
    }
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";
}

void cmdList() {
    ensureDir();
    std::vector<std::string> sites;
    for (auto& entry : fs::directory_iterator(SITES_DIR)) {
        sites.push_back(entry.path().filename().string());
    }
    if (sites.empty()) {
        std::cout << "No sites yet. Use 'create <name>.<domain>' to make one.\n";
        return;
    }
    std::sort(sites.begin(), sites.end());
    std::cout << "Sites on fake-web:\n";
    for (auto& s : sites) {
        std::cout << "  • " << s << "\n";
    }
}

void cmdDelete(const std::string& address) {
    std::string path = sitePath(address);
    if (!fs::exists(path)) {
        std::cout << "Site '" << address << "' not found.\n";
        return;
    }
    fs::remove(path);
    std::cout << "Deleted '" << address << "'.\n";
}

void printHelp() {
    std::cout << "\n  fake-web - your personal fake internet\n\n";
    std::cout << "  Commands:\n";
    std::cout << "    create <name>.<domain>   Create a new site (e.g. create hello.com)\n";
    std::cout << "    edit   <name>.<domain>   Add/replace content of a site\n";
    std::cout << "    visit  <name>.<domain>   View the site contents\n";
    std::cout << "    list                     List all sites\n";
    std::cout << "    delete <name>.<domain>   Delete a site\n";
    std::cout << "    help                     Show this help\n";
    std::cout << "    exit                     Quit fake-web\n\n";
    std::cout << "  Domains can be anything: .com .net .pizza .lol .whatever\n\n";
}

int main() {
    ensureDir();
    std::cout << "Welcome to fake-web! Type 'help' for commands.\n";

    std::string input;
    while (true) {
        std::cout << "fake-web> ";
        if (!std::getline(std::cin, input)) break;

        // trim
        while (!input.empty() && isspace(input.front())) input.erase(input.begin());
        while (!input.empty() && isspace(input.back())) input.pop_back();

        if (input.empty()) continue;

        std::istringstream iss(input);
        std::string cmd, arg;
        iss >> cmd;
        std::getline(iss >> std::ws, arg);

        if (cmd == "exit" || cmd == "quit") {
            std::cout << "Goodbye!\n";
            break;
        } else if (cmd == "help") {
            printHelp();
        } else if (cmd == "create") {
            if (arg.empty()) std::cout << "Usage: create <name>.<domain>\n";
            else cmdCreate(arg);
        } else if (cmd == "edit") {
            if (arg.empty()) std::cout << "Usage: edit <name>.<domain>\n";
            else cmdEdit(arg);
        } else if (cmd == "visit") {
            if (arg.empty()) std::cout << "Usage: visit <name>.<domain>\n";
            else cmdVisit(arg);
        } else if (cmd == "list") {
            cmdList();
        } else if (cmd == "delete" || cmd == "rm") {
            if (arg.empty()) std::cout << "Usage: delete <name>.<domain>\n";
            else cmdDelete(arg);
        } else {
            std::cout << "Unknown command: '" << cmd << "'. Type 'help' for commands.\n";
        }
    }
    return 0;
}
