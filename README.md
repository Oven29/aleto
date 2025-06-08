# ALETO

A universal application for viewing database tables (sqlite, postgresql, mysql in the future)

### Installation guide

- [Download WxWidgets 3.2.8](https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.8/wxWidgets-3.2.8.zip) from [official site](https://wxwidgets.org/downloads/) and unpack archive to `libs/wxWidgets-3.2.8`

- **Install lib for postgresql:**
***Install on linux (Ubuntu):***
```bash
sudo apt update
sudo apt install -y libpqxx-dev libpq-dev
```
***Install on linux (Arch):***
```bash
sudo pacman -S libpqxx
```
***Install on linux (Fedora, use dnf):***
```bash
sudo dnf install libpqxx-devel postgresql-devel
```
***Install on MacOS:***
```bash
brew install libpqxx
brew install libpq
```

- **Build with CMake project using commands:**
```bash
mkdir build
cd build
cmake ..
cmake --build .
```
- **Run:**
```bash
./aleto
```
