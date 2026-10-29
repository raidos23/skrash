<p align="center">      
  <img src="https://raw.githubusercontent.com/raidos23/skrash/main/docs/img/skrash.png" alt="skrash logo" width="80%"/>      
</p>      
      
# SKRASH     
      
**skrash** is a native Scratch 3.0 (`.sb3`) project runtime and executor.      
             
## Requirements      
      
- `libzip`      
- `SDL2`      
      
## Build & Run      
      
### Tests      
      
```bash      
make ctest      # C tests      
make pytest     # Python tests      
make clean  
```    
  
      
### Skrash Player         
```bash      
make play      
./build/skrash-play path/to/project.sb3      
```
      
## Python API (example)      
```python     
import skrash

# Load a Scratch project
project = skrash.load("game.sb3")

# Access and modify sprites
player = project.get_sprite("Player")
player.x = 100
player.y = 50
player.say("Hello from Python!")

# Run the project (opens the SDL2 window)
project.run()
```   
  
      
## Status      
      
See **[Todo.md](Todo.md)** for the detailed list of implemented Scratch blocks and remaining work.      
      
## License

licensed under the MIT License