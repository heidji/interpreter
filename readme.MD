## Installation / How to build
```bash
sudo apt-get install php7.1-dev
git clone https://github.com/CopernicaMarketingSoftware/PHP-CPP.git
make
sudo make install
git clone https://github.com/heidji/interpreter.git
cd interpreter
make -B
```

The generated `interpreter.so` file can included as follows:
Add an `interpreter`.ini file in the `/etc/php/7.3/mods-available` folder with the following content
`extension=/full/path/to/so/example.so`