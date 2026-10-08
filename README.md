# Week 1 Advanced Programming

Zoe Efstathiou

2423029

Itch Page: https://princessbleach.itch.io/gatcha-cat-wars


*Note, Ignore CPP files, these were made before writing the C files as a mistake.*

## The Gatcha Cat Fighter

I made a cat-collecting and battling game developed using C and raylib. The game features different cat rarities, a collection system, biscuit rewards, and battles.

Players sart with 10 biscuits and spend one biscuit to "roll" to collect different cat breeds (which are randomly selected from the cat breed database API). Some are rarer than others and the number of cats, unique breeds and biscuits are recorded. Higher rarity cats have better health, attacks and higher biscuit rewards. If the player collects a rarer cat, they are rewarded with biscuits.

The battle aspect is turn-based, with three options: attack, special and defend. Special abilities are dependent on cat rarity. 

### How to Compile with Emscripten and play in browser

The game can be compiled for web browsers using Emscripten. Emscripten is an open source compiler which can help convert languages such as C into WebAssembly. 

First, activate the Emscripten environment:

```bash
source ~/emsdk/emsdk_env.sh
```

Navigate to the project directory and run the following command:

```bash
emcc main.c -o web/index.html \
  -std=c99 \
  -I$HOME/raylib-web/src \
  $HOME/raylib-web/src/libraylib.web.a \
  -sUSE_GLFW=3 \
  -sASYNCIFY \
  -sFETCH=1 \
  -sALLOW_MEMORY_GROWTH=1 \
  --shell-file $HOME/raylib-web/src/shell.html
```

This generates the HTML, JavaScript, and WebAssembly files required to run the game in a browser. 


To test the game locally, run the following command from the project directory:

```bash
python3 -m http.server 8000 --directory web
```

Open the following address in a web browser:

http://localhost:8000

The game can then be played directly in the browser.



### API Source

The game uses **TheCatAPI** to retrieve cat breed information.

Website: https://thecatapi.com/

API endpoint:

https://api.thecatapi.com/v1/breeds

The game sends a GET request to retrieve a list of cat breeds and uses the returned information to generate cats within the game.

The request uses an API key supplied through the `x-api-key` header.

### Critical Reflection

My biggest challenge was trying to create a unique game that was easily made and could implement an API well. I originally thought of implementing a weather system into a simple object collection game but found this was rather boring. I looked for other API's online and found one which contained a library of different cat breeds. I realised I could use this to implement a random roll Gatcha system. After testing the game and receiving feedback, I decided that the game was too simple in it's current state and needed another element. I added a combat system and connected the combat abilities to the cat breed - this gave incentive for players to roll for different cats. 



### AI DECLARATION

I used ChatGPT (OpenAI) as an AI assistance tool during the development of The Cat Distribution System.

AI assistance was used to help write the game code and provide guidance on using Emscripten to create a browser-compatible build.

I reviewed and tested the generated code, including compiling the game and checking its functionality in both desktop and browser environments.

The creative direction, game concept, design decisions, and final implementation were guided by my own work and judgement. 

AI tool used: ChatGPT (OpenAI)
