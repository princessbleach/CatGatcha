#include "raylib.h"
#include <string>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <emscripten/fetch.h>
#include <cstring>






// PINK COLOUR PALETTE
const Color BACKGROUND = {255, 240, 245, 255};
const Color CUSTOM_PINK = {248, 187, 208, 255};
const Color DARK_PINK = {244, 143, 177, 255};
const Color BERRY = {173, 20, 87, 255};
const Color CREAM = {255, 248, 240, 255};
const Color CUSTOM_BROWN = {93, 64, 55, 255};
const Color LIGHT_PINK = {255, 218, 230, 255};
const Color PALE_PURPLE = {237, 231, 246, 255};
const Color CUSTOM_WHITE = {255, 255, 255, 255};
const Color CUSTOM_BLACK = {0, 0, 0, 255};

// ==========================================
// RARITY SYSTEM
// ==========================================

enum Rarity { COMMON, UNCOMMON, RARE, LEGENDARY, MYTHICAL };

struct RarityData {
    std::string name;
    Color colour;
    int biscuitReward;
    int maxHealth;
    int attack;
};

RarityData GetRarityData(Rarity rarity) {
    switch (rarity) {
        case COMMON:    return {"Common", GREEN, 1, 70, 10};
        case UNCOMMON:  return {"Uncommon", SKYBLUE, 2, 85, 13};
        case RARE:      return {"Rare", PURPLE, 5, 100, 15};
        case LEGENDARY: return {"Legendary", GOLD, 10, 115, 18};
        case MYTHICAL:  return {"Mythical", MAGENTA, 25, 130, 22};
        default:        return {"Common", GREEN, 1, 70, 10};
    }
}

Rarity RollRarity() {
    int roll = GetRandomValue(1, 100);
    if (roll <= 60) return COMMON;
    if (roll <= 85) return UNCOMMON;
    if (roll <= 95) return RARE;
    if (roll <= 99) return LEGENDARY;
    return MYTHICAL;
}

// ==========================================
// CAT API - WEB VERSION
// ==========================================


std::vector<std::string> breedNames;

bool breedsLoaded = false;
bool breedsFailed = false;

// Pick a random breed from the data received from TheCatAPI
struct CatData
{
    std::string breed;
};
CatData FetchCat()
{
    CatData cat;
    cat.breed = "Mystery Cat";

    if (!breedNames.empty())
    {
        int randomIndex = GetRandomValue(
            0,
            (int)breedNames.size() - 1
        );

        cat.breed = breedNames[randomIndex];
    }

    return cat;
}

// Called automatically when the API request succeeds
void OnBreedsLoaded(emscripten_fetch_t* fetch)
{
    std::string response(fetch->data, fetch->numBytes);

    size_t pos = 0;

    while ((pos = response.find("\"name\"", pos)) != std::string::npos)
    {
        pos = response.find(':', pos);

        if (pos == std::string::npos)
            break;

        pos++;

        while (pos < response.length() &&
            (response[pos] == ' ' || response[pos] == '\t'))
        {
            pos++;
        }

        if (pos >= response.length() || response[pos] != '"')
            continue;

        pos++;

        size_t end = response.find('"', pos);

        if (end == std::string::npos)
            break;

        std::string name = response.substr(pos, end - pos);

        if (!name.empty())
            breedNames.push_back(name);

        pos = end + 1;
    }

    breedsLoaded = !breedNames.empty();
    breedsFailed = !breedsLoaded;

    emscripten_fetch_close(fetch);
}

// Called if the API request fails
void OnBreedsFailed(emscripten_fetch_t* fetch)
{
    breedsFailed = true;

    emscripten_fetch_close(fetch);
}

// Start the API request without freezing the game
void LoadCatBreeds()
{
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);

    strcpy(attr.requestMethod, "GET");
    static const char* headers[] = {
    "x-api-key",
    "live_zFdcEMaKv1YFR7Sc3O8Ykm9I34I6QYsmU8am0KMFqqLZR9Xsk57DnmQomT6FbG6C",
    nullptr
};

attr.requestHeaders = headers;

    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

    attr.onsuccess = OnBreedsLoaded;
    attr.onerror = OnBreedsFailed;

    emscripten_fetch(
        &attr,
        "https://api.thecatapi.com/v1/breeds?limit=1000"
    );
}
// ==========================================
// CAT DRAWING
// ==========================================

void DrawCat(int x, int y, Color furColour, float scale) {
    // Ears
    DrawTriangle(
        {(float)x - 65 * scale, (float)y - 25 * scale},
        {(float)x - 55 * scale, (float)y - 105 * scale},
        {(float)x - 5 * scale, (float)y - 75 * scale}, furColour);
    DrawTriangle(
        {(float)x + 5 * scale, (float)y - 75 * scale},
        {(float)x + 55 * scale, (float)y - 105 * scale},
        {(float)x + 65 * scale, (float)y - 25 * scale}, furColour);

    // Inner ears
    DrawTriangle(
        {(float)x - 52 * scale, (float)y - 48 * scale},
        {(float)x - 55 * scale, (float)y - 88 * scale},
        {(float)x - 25 * scale, (float)y - 65 * scale}, DARK_PINK);
    DrawTriangle(
        {(float)x + 25 * scale, (float)y - 65 * scale},
        {(float)x + 55 * scale, (float)y - 88 * scale},
        {(float)x + 52 * scale, (float)y - 48 * scale}, DARK_PINK);

    // Face
    DrawCircle(x, y, 75 * scale, furColour);
    DrawCircle(x - 27 * scale, y - 5 * scale, 5 * scale, BLACK);
    DrawCircle(x + 27 * scale, y - 5 * scale, 5 * scale, BLACK);

    // Nose and mouth
    DrawTriangle(
        {(float)x - 7 * scale, (float)y + 12 * scale},
        {(float)x + 7 * scale, (float)y + 12 * scale},
        {(float)x, (float)y + 20 * scale}, DARK_PINK);
    DrawLineEx({(float)x, (float)y + 20 * scale},
               {(float)x, (float)y + 27 * scale}, 2 * scale, BLACK);
    DrawLineEx({(float)x, (float)y + 27 * scale},
               {(float)x - 9 * scale, (float)y + 32 * scale}, 2 * scale, BLACK);
    DrawLineEx({(float)x, (float)y + 27 * scale},
               {(float)x + 9 * scale, (float)y + 32 * scale}, 2 * scale, BLACK);

    // Whiskers
    DrawLineEx({(float)x - 35 * scale, (float)y + 15 * scale},
               {(float)x - 85 * scale, (float)y + 5 * scale}, 2 * scale, CUSTOM_BROWN);
    DrawLineEx({(float)x - 35 * scale, (float)y + 25 * scale},
               {(float)x - 85 * scale, (float)y + 30 * scale}, 2 * scale, CUSTOM_BROWN);
    DrawLineEx({(float)x + 35 * scale, (float)y + 15 * scale},
               {(float)x + 85 * scale, (float)y + 5 * scale}, 2 * scale, CUSTOM_BROWN);
    DrawLineEx({(float)x + 35 * scale, (float)y + 25 * scale},
               {(float)x + 85 * scale, (float)y + 30 * scale}, 2 * scale, CUSTOM_BROWN);

    // Cheeks
    DrawCircle(x - 43 * scale, y + 22 * scale, 9 * scale, LIGHT_PINK);
    DrawCircle(x + 43 * scale, y + 22 * scale, 9 * scale, LIGHT_PINK);
}

// ==========================================
// UI HELPERS
// ==========================================

bool Button(Rectangle bounds, const char* label, Color normal, Color hover) {
    Vector2 mouse = GetMousePosition();
    bool hovering = CheckCollisionPointRec(mouse, bounds);
    DrawRectangleRounded(bounds, 0.25f, 10, hovering ? hover : normal);
    int textWidth = MeasureText(label, 20);
    DrawText(label, (int)(bounds.x + (bounds.width - textWidth) / 2),
             (int)(bounds.y + (bounds.height - 20) / 2), 20, BERRY);
    return hovering && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void DrawHitBurst(int x, int y, float timeLeft, Color colour) {
    // A quick ring of stars that expands and fades after a hit.
    float progress = 1.0f - (timeLeft / 0.42f);
    float radius = 12.0f + progress * 42.0f;
    unsigned char alpha = (unsigned char)(255.0f * (1.0f - progress));
    Color faded = {colour.r, colour.g, colour.b, alpha};

    for (int i = 0; i < 8; ++i) {
        float angle = (float)i * (PI / 4.0f);
        float px = x + cosf(angle) * radius;
        float py = y + sinf(angle) * radius;
        DrawPoly({px, py}, 4, 5.0f * (1.0f - progress * 0.35f), 45.0f, faded);
    }

    DrawCircleLines(x, y, radius * 0.65f, faded);
}

void DrawHealthBar(int x, int y, int width, int health, int maxHealth) {
    DrawRectangleRounded({(float)x, (float)y, (float)width, 18}, 0.4f, 8, LIGHTGRAY);

    float ratio = maxHealth > 0 ? (float)health / maxHealth : 0.0f;
    ratio = std::max(0.0f, std::min(1.0f, ratio));

    if (ratio > 0.0f) {
        DrawRectangleRounded({(float)x, (float)y, width * ratio, 18}, 0.4f, 8,
                             ratio > 0.5f ? GREEN : (ratio > 0.25f ? ORANGE : RED));
    }

    DrawText(TextFormat("%i / %i", health, maxHealth), x, y + 22, 16, CUSTOM_BROWN);
}

// ==========================================
// GAME STATES
// ==========================================

enum GameScreen { COLLECTION, BATTLE, RESULTS };

struct Cat {
    std::string breed;
    Rarity rarity;
    Color furColour;
    int health;
    int maxHealth;
    int attack;
};

Cat MakeCat(const std::string& breed, Rarity rarity, Color furColour) {
    RarityData stats = GetRarityData(rarity);
    return {breed, rarity, furColour, stats.maxHealth, stats.maxHealth, stats.attack};
}

int main() {
    InitWindow(800, 600, "The Cat Distribution System");
    SetTargetFPS(60);
    SetRandomSeed((unsigned int)time(nullptr));
    // Load cat breeds from TheCatAPI
    LoadCatBreeds();

    int biscuits = 10;
    int catsCollected = 0;
    int uniqueCats = 0;
    std::map<std::string, int> collection;
    std::vector<Cat> cats;

    Color catColours[] = {CUSTOM_PINK, CREAM, ORANGE, CUSTOM_BROWN, LIGHTGRAY};
    GameScreen screen = COLLECTION;

    Cat playerCat = MakeCat("Your Cat", COMMON, CUSTOM_PINK);
    Cat enemyCat = MakeCat("Rival Cat", COMMON, LIGHTGRAY);

    bool hasCat = false;
    bool playerDefending = false;
    bool enemyDefending = false;
    bool playerWon = false;

    std::string message = "Click below to find a furry friend!";
    std::string battleMessage = "A rival cat appears!";
    std::string resultMessage = "";

    // Battle animation state. The numbers are kept separate from combat logic
    // so the cats can animate after damage has been calculated.
    float attackAnimTime = 0.0f;
    float hitAnimTime = 0.0f;
    float defendAnimTime = 0.0f;
    float floatingTextTime = 0.0f;
    int floatingDamage = 0;
    bool floatingOnEnemy = true;
    std::string floatingLabel = "";
    bool specialAttack = false;

    Rectangle rollButton = {250, 480, 300, 65};

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();
        float dt = GetFrameTime();

        attackAnimTime = std::max(0.0f, attackAnimTime - dt);
        hitAnimTime = std::max(0.0f, hitAnimTime - dt);
        defendAnimTime = std::max(0.0f, defendAnimTime - dt);
        floatingTextTime = std::max(0.0f, floatingTextTime - dt);

        BeginDrawing();
        ClearBackground(BACKGROUND);

        if (screen == COLLECTION) {
            DrawText("THE CAT DISTRIBUTION SYSTEM", 125, 35, 30, BERRY);
            if (!breedsLoaded)
            {
                if (breedsFailed)
                {
                    DrawText("Couldn't connect to TheCatAPI. Please refresh the page.", 100, 90, 18, RED);
                    }
                    else
                    {
                        DrawText("Finding the perfect cats for you...", 205, 90, 18, CUSTOM_BROWN);}
                        }

            DrawText(TextFormat("Fish Biscuits: %i", biscuits), 40, 125, 22, BERRY);
            DrawText(TextFormat("Cats: %i", catsCollected), 520, 125, 22, BERRY);
            DrawText(TextFormat("Unique: %i", uniqueCats), 520, 150, 18, CUSTOM_BROWN);

            DrawRectangleRounded({200, 185, 400, 260}, 0.15f, 10, CUSTOM_WHITE);

            if (hasCat) {
                DrawCat(400, 290, playerCat.furColour, 1.0f);

                DrawText(playerCat.breed.c_str(),
                         400 - MeasureText(playerCat.breed.c_str(), 22) / 2,
                         380, 22, BERRY);

                RarityData rarityInfo = GetRarityData(playerCat.rarity);

                DrawText(rarityInfo.name.c_str(),
                         400 - MeasureText(rarityInfo.name.c_str(), 20) / 2,
                         410, 20, rarityInfo.colour);
            } else {
                DrawText("Your cat is waiting...", 270, 280, 25, DARK_PINK);
                DrawText("Give them a home!", 310, 320, 20, CUSTOM_BROWN);
            }

            DrawText(message.c_str(), 150, 455, 18, BERRY);

            bool rollHover = breedsLoaded &&
            CheckCollisionPointRec(mouse, rollButton);

            DrawRectangleRounded(rollButton, 0.3f, 10,
                                 rollHover ? DARK_PINK : CUSTOM_PINK);

            DrawText("ROLL FOR CAT!", 310, 498, 25, BERRY);
            
            if (hasCat && breedsLoaded && Button({250, 550, 300, 40},
                                 "BATTLE A RIVAL CAT",
                                 PALE_PURPLE, CUSTOM_PINK)) {

                Rarity enemyRarity = RollRarity();
                int enemyColour = GetRandomValue(0, 4);
                CatData rival = FetchCat();

                enemyCat = MakeCat(
                    rival.breed == "Mystery Cat" ? "Rival Cat" : rival.breed,
                    enemyRarity,
                    catColours[enemyColour]
                );

                enemyCat.health = enemyCat.maxHealth;
                playerCat.health = playerCat.maxHealth;

                playerDefending = false;
                enemyDefending = false;
                battleMessage = "A rival cat appears!";
                screen = BATTLE;
            }

            

            if (rollHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (biscuits > 0) {
                    biscuits--;
                    message = "Searching for your new friend...";

                    CatData found = FetchCat();
                    Rarity rarity = RollRarity();
                    int colourIndex = GetRandomValue(0, 4);

                    playerCat = MakeCat(found.breed, rarity, catColours[colourIndex]);

                    bool isNewCat = (collection[playerCat.breed] == 0);
                    collection[playerCat.breed]++;
                    catsCollected++;

                    RarityData reward = GetRarityData(rarity);

                    if (isNewCat) {
                        uniqueCats++;
                        biscuits += reward.biscuitReward;
                        message = "A new friend! +" +
                                  std::to_string(reward.biscuitReward) +
                                  " biscuits!";
                    } else {
                        biscuits++;
                        message = "Duplicate cat! +1 biscuit!";
                    }

                    hasCat = true;
                } else {
                    message = "Oh no! You're out of fish biscuits!";
                }
            }
        }
        else if (screen == BATTLE) {
            DrawText("CAT BATTLE!", 300, 30, 32, BERRY);
            DrawText("Your cat", 130, 105, 22, CUSTOM_BROWN);
            DrawText("Rival cat", 550, 105, 22, CUSTOM_BROWN);

            // Idle bobbing, a forward lunge, and a tiny recoil make the cats feel alive.
            float idleBob = sinf((float)GetTime() * 4.0f) * 4.0f;

            float lunge = attackAnimTime > 0.0f
                ? sinf((1.0f - attackAnimTime / 0.42f) * PI) * 62.0f
                : 0.0f;

            float recoil = hitAnimTime > 0.0f
                ? sinf((1.0f - hitAnimTime / 0.28f) * PI * 5.0f) * 7.0f
                : 0.0f;

            float playerY = 245.0f + idleBob;
            float enemyY = 245.0f + sinf((float)GetTime() * 4.0f + 1.0f) * 4.0f;

            int playerX = (int)(220.0f + lunge);
            int enemyX = (int)(580.0f + recoil);

            // Soft arena shadows and a dashed centre line.
            DrawEllipse(playerX, 316, 72, 12, LIGHT_PINK);
            DrawEllipse(enemyX, 316, 72, 12, LIGHT_PINK);

            for (int y = 150; y < 315; y += 24) {
                DrawLine(400, y, 400, y + 10, Fade(CUSTOM_PINK, 0.65f));
            }

            DrawCat(playerX, (int)playerY, playerCat.furColour, 0.9f);
            DrawCat(enemyX, (int)enemyY, enemyCat.furColour, 0.9f);

            if (defendAnimTime > 0.0f) {
                DrawCircleLines(
                    playerX,
                    (int)playerY,
                    91.0f + sinf((float)GetTime() * 18.0f) * 4.0f,
                    SKYBLUE
                );

                DrawText("SHIELD!", playerX - 48, (int)playerY - 112, 20, SKYBLUE);
            }

            if (attackAnimTime > 0.0f) {
                DrawHitBurst(
                    enemyX - 20,
                    (int)enemyY + 8,
                    attackAnimTime,
                    specialAttack ? MAGENTA : DARK_PINK
                );
            }

            if (hitAnimTime > 0.0f) {
                DrawHitBurst(
                    playerX + 20,
                    (int)playerY + 8,
                    hitAnimTime,
                    ORANGE
                );
            }

            if (floatingTextTime > 0.0f) {
                float rise = (1.0f - floatingTextTime / 0.85f) * 34.0f;

                int fx = floatingOnEnemy ? enemyX : playerX;
                int fy = (int)(floatingOnEnemy ? enemyY : playerY) - 100 - (int)rise;

                Color textColour = floatingLabel == "BLOCKED!"
                    ? SKYBLUE
                    : (floatingOnEnemy ? BERRY : ORANGE);

                DrawText(
                    floatingLabel.c_str(),
                    fx - MeasureText(floatingLabel.c_str(), 24) / 2,
                    fy,
                    24,
                    textColour
                );
            }

            DrawHealthBar(105, 340, 230, playerCat.health, playerCat.maxHealth);
            DrawHealthBar(465, 340, 230, enemyCat.health, enemyCat.maxHealth);

            RarityData playerRarity = GetRarityData(playerCat.rarity);
            RarityData enemyRarity = GetRarityData(enemyCat.rarity);

            DrawText(playerCat.breed.c_str(), 105, 390, 18, BERRY);
            DrawText(playerRarity.name.c_str(), 105, 414, 16, playerRarity.colour);

            DrawText(enemyCat.breed.c_str(), 465, 390, 18, BERRY);
            DrawText(enemyRarity.name.c_str(), 465, 414, 16, enemyRarity.colour);

            DrawRectangleRounded({90, 445, 620, 55}, 0.15f, 8, CUSTOM_WHITE);
            DrawText(battleMessage.c_str(), 110, 463, 18, BERRY);

            // Prevent another move while the current hit animation is playing.
            bool animating = attackAnimTime > 0.0f || hitAnimTime > 0.0f;

            bool attackClicked = !animating &&
                Button({65, 520, 200, 55}, "ATTACK", CUSTOM_PINK, DARK_PINK);

            bool specialClicked = !animating &&
                Button({300, 520, 200, 55}, "SPECIAL", PALE_PURPLE, CUSTOM_PINK);

            bool defendClicked = !animating &&
                Button({535, 520, 200, 55}, "DEFEND", CREAM, CUSTOM_PINK);

            if (animating) {
                DrawRectangleRounded(
                    {65, 520, 200, 55}, 0.25f, 10,
                    Fade(CUSTOM_PINK, 0.55f)
                );

                DrawRectangleRounded(
                    {300, 520, 200, 55}, 0.25f, 10,
                    Fade(PALE_PURPLE, 0.55f)
                );

                DrawRectangleRounded(
                    {535, 520, 200, 55}, 0.25f, 10,
                    Fade(CREAM, 0.55f)
                );

                DrawText("ATTACK", 128, 538, 20, Fade(BERRY, 0.5f));
                DrawText("SPECIAL", 355, 538, 20, Fade(BERRY, 0.5f));
                DrawText("DEFEND", 592, 538, 20, Fade(BERRY, 0.5f));
            }

            bool actionClicked = attackClicked || specialClicked || defendClicked;

            if (actionClicked) {
                int damage = 0;
                std::string ability;

                if (attackClicked) {
                    damage = playerCat.attack;
                    ability = "Claw swipe!";
                }
                else if (specialClicked) {
                    switch (playerCat.rarity) {
                        case COMMON:
                            damage = 14;
                            ability = "Scratch Attack!";
                            break;

                        case UNCOMMON:
                            damage = 18;
                            ability = "Double Pounce!";
                            break;

                        case RARE:
                            playerCat.health = std::min(
                                playerCat.maxHealth,
                                playerCat.health + 20
                            );
                            damage = 8;
                            ability = "Nine Lives! +20 HP";
                            break;

                        case LEGENDARY:
                            damage = 25;
                            ability = "Royal Decree!";
                            break;

                        case MYTHICAL:
                            damage = 35;
                            ability = "CAT-ASTROPHE!";
                            break;
                    }

                    specialAttack = true;
                }
                else {
                    playerDefending = true;
                    defendAnimTime = 0.6f;
                    battleMessage = playerCat.breed + " is bracing for impact!";

                    floatingLabel = "SHIELD UP!";
                    floatingTextTime = 0.85f;
                    floatingOnEnemy = false;
                }

                if (!defendClicked) {
                    if (enemyDefending) {
                        damage = (damage + 1) / 2;
                    }

                    enemyCat.health = std::max(0, enemyCat.health - damage);
                    enemyDefending = false;

                    attackAnimTime = specialAttack ? 0.58f : 0.42f;

                    floatingDamage = damage;
                    floatingLabel = "-" + std::to_string(damage) + " HP!";
                    floatingTextTime = 0.85f;
                    floatingOnEnemy = true;

                    battleMessage = ability + " " +
                                    std::to_string(damage) + " damage!";

                    specialAttack = false;
                }

                // The rival responds after the player's move. The hit animation
                // plays while the next action buttons are temporarily disabled.
                if (enemyCat.health <= 0) {
                    playerWon = true;
                    resultMessage = "Victory! Your cat won the battle!";
                    screen = RESULTS;
                }
                else {
                    int enemyDamage = enemyCat.attack;

                    if (playerDefending) {
                        enemyDamage = (enemyDamage + 1) / 2;
                    }

                    playerCat.health = std::max(
                        0,
                        playerCat.health - enemyDamage
                    );

                    hitAnimTime = 0.28f;
                    floatingDamage = enemyDamage;

                    if (playerDefending) {
                        floatingLabel = "BLOCKED!";
                        floatingOnEnemy = false;
                    }
                    else {
                        floatingLabel = "-" +
                                        std::to_string(enemyDamage) +
                                        " HP!";
                        floatingOnEnemy = false;
                    }

                    floatingTextTime = 0.85f;
                    playerDefending = false;

                    if (playerCat.health <= 0) {
                        playerWon = false;
                        resultMessage = "Oh no! Your cat needs a nap.";
                        screen = RESULTS;
                    }
                    else {
                        battleMessage += " Rival cat hits back for " +
                                         std::to_string(enemyDamage) + "!";
                    }
                }
            }
        }
        else if (screen == RESULTS) {
            DrawText(
                playerWon ? "VICTORY!" : "BATTLE OVER",
                270, 100, 40, BERRY
            );

            DrawCat(400, 275, playerCat.furColour, 1.0f);

            DrawText(
                resultMessage.c_str(),
                400 - MeasureText(resultMessage.c_str(), 20) / 2,
                390, 20, CUSTOM_BROWN
            );

            if (playerWon) {
                DrawText("You earned 5 fish biscuits!", 270, 425, 20, BERRY);
            }

            if (Button(
                {250, 485, 300, 65},
                "BACK TO COLLECTION",
                CUSTOM_PINK,
                DARK_PINK
            )) {
                if (playerWon) {
                    biscuits += 5;
                }

                playerCat.health = playerCat.maxHealth;
                screen = COLLECTION;

                message = playerWon
                    ? "A victory! +5 biscuits!"
                    : "Rest up and try again!";
            }
        }

        EndDrawing();
    }

   
    CloseWindow();

    return 0;
}