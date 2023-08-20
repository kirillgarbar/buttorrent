# buttorrent
Simple BitTorrent client made for practice.

 Quick start
==========================
To download your torrent file you can follow these steps.
- Clone the repository and build source files:
```console
git clone https://github.com/kirillgarbar/buttorrent
cd buttorrent && mkdir build && cd build
cmake .. && cmake --build .
```
- Then you can download the example torrent file:
```console
./Buttorrent -f ../../result/ComputerNetworks.torrent -o ../../result/
```

Commandline Options
==========================
The program supports the following commandline options:

| Options | Alternative    | Description                                                                                        | Default            |
|---------|----------------|----------------------------------------------------------------------------------------------------|--------------------|
| -f      | --torrentFile | Path to the Torrent file                                                                           | REQUIRED           |
| -o      | --outputDirectory   | The output directory to which the file will be downloaded                                    | result/           |
