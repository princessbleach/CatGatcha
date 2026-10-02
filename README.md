
# The Gatcha Cat Fighter

I made a cat-collecting and battling game developed using C and raylib. The game features different cat rarities, a collection system, biscuit rewards, and battles.

Players sart with 10 biscuits and spend one biscuit to "roll" to collect different cat breeds (which are randomly selected from the cat breed database API). Some are rarer than others and the number of cats, unique breeds and biscuits are recorded. Higher rarity cats have better health, attacks and higher biscuit rewards. If the player collects a rarer cat, they are rewarded with biscuits.

The battle aspect is turn-based, with three options: attack, special and defend. Special abilities are dependent on cat rarity. 

## How to Compile with Emscripten

The game can be compiled for web browsers using Emscripten.

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

## How to Serve and Play in a Browser

To test the game locally, run the following command from the project directory:

```bash
python3 -m http.server 8000 --directory web
```

Open the following address in a web browser:

http://localhost:8000

The game can then be played directly in the browser.

To stop the local server, press `Control + C` in the Terminal.

## API Source

The game uses **TheCatAPI** to retrieve cat breed information.

Website: https://thecatapi.com/

API endpoint:

https://api.thecatapi.com/v1/breeds

The game sends a GET request to retrieve a list of cat breeds and uses the returned information to generate cats within the game.

The request uses an API key supplied through the `x-api-key` header.

## Technologies Used

- C
- raylib
- Emscripten
- WebAssembly
- TheCatAPI
- Python HTTP Server


## AI DECLARATION

I used ChatGPT (OpenAI) as an AI assistance tool during the development of The Cat Distribution System.

AI assistance was used to help convert the original C++ code into C, troubleshoot compilation issues, provide guidance on using Emscripten to create a browser-compatible build, and assist with writing project documentation.

I reviewed and tested the generated code, including compiling the game and checking its functionality in both desktop and browser environments.

The creative direction, game concept, design decisions, and final implementation were guided by my own work and judgement. 

AI tool used: ChatGPT (OpenAI)
Purpose: Programming assistance, debugging, code conversion, and documentation.