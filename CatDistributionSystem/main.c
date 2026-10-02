#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/fetch.h>
#endif
#ifndef __EMSCRIPTEN__
#include <curl/curl.h>
#endif

#define MAX_BREEDS 300
#define BREED_LENGTH 96
#define MAX_CATS 300
#define PI_F 3.14159265358979323846f


#define CAT_API_KEY "live_zFdcEMaKv1YFR7Sc3O8Ykm9I34I6QYsmU8am0KMFqqLZR9Xsk57DnmQomT6FbG6C"

typedef enum { COMMON, UNCOMMON, RARE, LEGENDARY, MYTHICAL } Rarity;
typedef enum { COLLECTION, BATTLE, RESULTS } GameScreen;
typedef struct { char name[20]; Color colour; int biscuitReward, maxHealth, attack; } RarityData;
typedef struct { char breed[BREED_LENGTH]; } CatData;
typedef struct { char breed[BREED_LENGTH]; Rarity rarity; Color furColour; int health, maxHealth, attack; } Cat;

static const Color BACKGROUND = {255,240,245,255};
static const Color CUSTOM_PINK = {248,187,208,255};
static const Color DARK_PINK = {244,143,177,255};
static const Color BERRY = {173,20,87,255};
static const Color CREAM = {255,248,240,255};
static const Color CUSTOM_BROWN = {93,64,55,255};
static const Color LIGHT_PINK = {255,218,230,255};
static const Color PALE_PURPLE = {237,231,246,255};
static const Color CUSTOM_WHITE = {255,255,255,255};

static char breedNames[MAX_BREEDS][BREED_LENGTH];
static int breedCount = 0;
static int breedsLoaded = 0;
static int breedsFailed = 0;

static RarityData GetRarityData(Rarity rarity) {
    switch (rarity) {
        case COMMON: return (RarityData){"Common", GREEN, 1, 70, 10};
        case UNCOMMON: return (RarityData){"Uncommon", SKYBLUE, 2, 85, 13};
        case RARE: return (RarityData){"Rare", PURPLE, 5, 100, 15};
        case LEGENDARY: return (RarityData){"Legendary", GOLD, 10, 115, 18};
        case MYTHICAL: return (RarityData){"Mythical", MAGENTA, 25, 130, 22};
        default: return (RarityData){"Common", GREEN, 1, 70, 10};
    }
}

static Rarity RollRarity(void) {
    int roll = GetRandomValue(1, 100);
    if (roll <= 60) return COMMON;
    if (roll <= 85) return UNCOMMON;
    if (roll <= 95) return RARE;
    if (roll <= 99) return LEGENDARY;
    return MYTHICAL;
}

static void ParseBreeds(const char *json) {
    const char *p = json;
    breedCount = 0;
    while ((p = strstr(p, "\"name\"")) != NULL && breedCount < MAX_BREEDS) {
        p = strchr(p, ':');
        if (!p) break;
        p++;
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
        if (*p != '"') continue;
        p++;
        const char *end = strchr(p, '"');
        if (!end) break;
        size_t n = (size_t)(end - p);
        if (n > 0 && n < BREED_LENGTH) {
            memcpy(breedNames[breedCount], p, n);
            breedNames[breedCount][n] = '\0';
            breedCount++;
        }
        p = end + 1;
    }
    breedsLoaded = breedCount > 0;
    breedsFailed = !breedsLoaded;
}

#ifndef __EMSCRIPTEN__
typedef struct { char *data; size_t length; } ResponseBuffer;
static size_t WriteResponse(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t bytes = size * nmemb;
    ResponseBuffer *buffer = (ResponseBuffer *)userp;
    char *next = realloc(buffer->data, buffer->length + bytes + 1);
    if (!next) return 0;
    buffer->data = next;
    memcpy(buffer->data + buffer->length, contents, bytes);
    buffer->length += bytes;
    buffer->data[buffer->length] = '\0';
    return bytes;
}
static void LoadCatBreeds(void) {
    CURL *curl = curl_easy_init();
    if (!curl) { breedsFailed = 1; return; }
    ResponseBuffer response = {calloc(1, 1), 0};
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.thecatapi.com/v1/breeds?limit=1000");
    char apiHeader[256];
    snprintf(apiHeader, sizeof(apiHeader), "x-api-key: %s", CAT_API_KEY);
    struct curl_slist *headers = curl_slist_append(NULL, apiHeader);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteResponse);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    CURLcode result = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    if (result == CURLE_OK && status == 200 && response.data) ParseBreeds(response.data);
    else breedsFailed = 1;
    free(response.data);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}
#else
static void OnBreedsLoaded(emscripten_fetch_t *fetch) {
    ParseBreeds(fetch->data);
    emscripten_fetch_close(fetch);
}
static void OnBreedsFailed(emscripten_fetch_t *fetch) {
    breedsFailed = 1;
    emscripten_fetch_close(fetch);
}
static void LoadCatBreeds(void) {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    const char *headers[] = {"x-api-key", CAT_API_KEY, NULL};
    attr.requestHeaders = headers;
    attr.onsuccess = OnBreedsLoaded;
    attr.onerror = OnBreedsFailed;
    emscripten_fetch(&attr, "https://api.thecatapi.com/v1/breeds?limit=1000");
}
#endif

static CatData FetchCat(void) {
    CatData cat = {{0}};
    strcpy(cat.breed, "Mystery Cat");
    if (breedCount > 0) {
        int index = GetRandomValue(0, breedCount - 1);
        strncpy(cat.breed, breedNames[index], BREED_LENGTH - 1);
        cat.breed[BREED_LENGTH - 1] = '\0';
    }
    return cat;
}

static void DrawCat(int x, int y, Color fur, float s) {
    DrawTriangle((Vector2){x-65*s,y-25*s},(Vector2){x-55*s,y-105*s},(Vector2){x-5*s,y-75*s},fur);
    DrawTriangle((Vector2){x+5*s,y-75*s},(Vector2){x+55*s,y-105*s},(Vector2){x+65*s,y-25*s},fur);
    DrawTriangle((Vector2){x-52*s,y-48*s},(Vector2){x-55*s,y-88*s},(Vector2){x-25*s,y-65*s},DARK_PINK);
    DrawTriangle((Vector2){x+25*s,y-65*s},(Vector2){x+55*s,y-88*s},(Vector2){x+52*s,y-48*s},DARK_PINK);
    DrawCircle(x,y,75*s,fur);
    DrawCircle(x-27*s,y-5*s,5*s,BLACK); DrawCircle(x+27*s,y-5*s,5*s,BLACK);
    DrawTriangle((Vector2){x-7*s,y+12*s},(Vector2){x+7*s,y+12*s},(Vector2){x,y+20*s},DARK_PINK);
    DrawLineEx((Vector2){x,y+20*s},(Vector2){x,y+27*s},2*s,BLACK);
    DrawLineEx((Vector2){x,y+27*s},(Vector2){x-9*s,y+32*s},2*s,BLACK);
    DrawLineEx((Vector2){x,y+27*s},(Vector2){x+9*s,y+32*s},2*s,BLACK);
    DrawLineEx((Vector2){x-35*s,y+15*s},(Vector2){x-85*s,y+5*s},2*s,CUSTOM_BROWN);
    DrawLineEx((Vector2){x-35*s,y+25*s},(Vector2){x-85*s,y+30*s},2*s,CUSTOM_BROWN);
    DrawLineEx((Vector2){x+35*s,y+15*s},(Vector2){x+85*s,y+5*s},2*s,CUSTOM_BROWN);
    DrawLineEx((Vector2){x+35*s,y+25*s},(Vector2){x+85*s,y+30*s},2*s,CUSTOM_BROWN);
    DrawCircle(x-43*s,y+22*s,9*s,LIGHT_PINK); DrawCircle(x+43*s,y+22*s,9*s,LIGHT_PINK);
}

static bool Button(Rectangle bounds, const char *label, Color normal, Color hover) {
    Vector2 mouse = GetMousePosition();
    bool hovering = CheckCollisionPointRec(mouse, bounds);
    DrawRectangleRounded(bounds,0.25f,10,hovering?hover:normal);
    int width = MeasureText(label,20);
    DrawText(label,(int)(bounds.x+(bounds.width-width)/2),(int)(bounds.y+(bounds.height-20)/2),20,BERRY);
    return hovering && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
static void DrawHitBurst(int x,int y,float timeLeft,Color colour) {
    float progress=1.0f-timeLeft/0.42f, radius=12.0f+progress*42.0f;
    Color faded={colour.r,colour.g,colour.b,(unsigned char)(255.0f*(1.0f-progress))};
    for(int i=0;i<8;i++){float a=i*(PI_F/4.0f);DrawPoly((Vector2){x+cosf(a)*radius,y+sinf(a)*radius},4,5.0f*(1.0f-progress*0.35f),45.0f,faded);}
    DrawCircleLines(x,y,radius*0.65f,faded);
}
static void DrawHealthBar(int x,int y,int width,int health,int maxHealth) {
    DrawRectangleRounded((Rectangle){x,y,width,18},0.4f,8,LIGHTGRAY);
    float ratio=maxHealth>0?(float)health/maxHealth:0.0f;
    if(ratio<0)ratio=0;if(ratio>1)ratio=1;
    if(ratio>0)DrawRectangleRounded((Rectangle){x,y,width*ratio,18},0.4f,8,ratio>0.5f?GREEN:(ratio>0.25f?ORANGE:RED));
    DrawText(TextFormat("%i / %i",health,maxHealth),x,y+22,16,CUSTOM_BROWN);
}
static Cat MakeCat(const char *breed,Rarity rarity,Color fur) {
    RarityData stats=GetRarityData(rarity); Cat cat={0};
    strncpy(cat.breed,breed,BREED_LENGTH-1);cat.rarity=rarity;cat.furColour=fur;
    cat.health=cat.maxHealth=stats.maxHealth;cat.attack=stats.attack;return cat;
}

int main(void) {
    InitWindow(800,600,"The Cat Distribution System"); SetTargetFPS(60); SetRandomSeed((unsigned int)time(NULL));
#ifndef __EMSCRIPTEN__
    curl_global_init(CURL_GLOBAL_DEFAULT);
#endif
    LoadCatBreeds();
    int biscuits=10,catsCollected=0,uniqueCats=0;
    char collectedBreeds[MAX_CATS][BREED_LENGTH]={{0}}; int collectedCounts[MAX_CATS]={0}; int collectedKinds=0;
    Color catColours[]={CUSTOM_PINK,CREAM,ORANGE,CUSTOM_BROWN,LIGHTGRAY};
    GameScreen screen=COLLECTION;
    Cat playerCat=MakeCat("Your Cat",COMMON,CUSTOM_PINK),enemyCat=MakeCat("Rival Cat",COMMON,LIGHTGRAY);
    bool hasCat=false,playerDefending=false,enemyDefending=false,playerWon=false;
    char message[180]="Click below to find a furry friend!",battleMessage[180]="A rival cat appears!",resultMessage[180]="";
    float attackAnimTime=0,hitAnimTime=0,defendAnimTime=0,floatingTextTime=0;int floatingDamage=0;bool floatingOnEnemy=true,specialAttack=false;char floatingLabel[64]="";
    Rectangle rollButton={250,480,300,65};

    while(!WindowShouldClose()) {
        Vector2 mouse=GetMousePosition();float dt=GetFrameTime();
        attackAnimTime=fmaxf(0,attackAnimTime-dt);hitAnimTime=fmaxf(0,hitAnimTime-dt);defendAnimTime=fmaxf(0,defendAnimTime-dt);floatingTextTime=fmaxf(0,floatingTextTime-dt);
        BeginDrawing();ClearBackground(BACKGROUND);
        if(screen==COLLECTION) {
            DrawText("THE CAT DISTRIBUTION SYSTEM",125,35,30,BERRY);
            DrawText(TextFormat("Fish Biscuits: %i",biscuits),40,125,22,BERRY);DrawText(TextFormat("Cats: %i",catsCollected),520,125,22,BERRY);DrawText(TextFormat("Unique: %i",uniqueCats),520,150,18,CUSTOM_BROWN);
            DrawRectangleRounded((Rectangle){200,185,400,260},0.15f,10,CUSTOM_WHITE);
            if(hasCat){DrawCat(400,290,playerCat.furColour,1.0f);DrawText(playerCat.breed,400-MeasureText(playerCat.breed,22)/2,380,22,BERRY);RarityData info=GetRarityData(playerCat.rarity);DrawText(info.name,400-MeasureText(info.name,20)/2,410,20,info.colour);}
            else {DrawText("Your cat is waiting...",270,280,25,DARK_PINK);DrawText("Give them a home!",310,320,20,CUSTOM_BROWN);}
            DrawText(message,150,455,18,BERRY);
            if(!breedsLoaded){DrawText(breedsFailed?"Couldn't connect to cat API":"Loading cat breeds...",265,465,16,RED);}
            bool rollHover=breedsLoaded&&CheckCollisionPointRec(mouse,rollButton);
            DrawRectangleRounded(rollButton,0.3f,10,rollHover?DARK_PINK:CUSTOM_PINK);DrawText("ROLL FOR CAT!",310,498,25,BERRY);
            if(hasCat&&breedsLoaded&&Button((Rectangle){250,550,300,40},"BATTLE A RIVAL CAT",PALE_PURPLE,CUSTOM_PINK)){
                Rarity er=RollRarity();int ec=GetRandomValue(0,4);CatData rival=FetchCat();enemyCat=MakeCat(strcmp(rival.breed,"Mystery Cat")==0?"Rival Cat":rival.breed,er,catColours[ec]);
                enemyCat.health=enemyCat.maxHealth;playerCat.health=playerCat.maxHealth;playerDefending=false;enemyDefending=false;strcpy(battleMessage,"A rival cat appears!");screen=BATTLE;
            }
            if(rollHover&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                if(biscuits>0){biscuits--;strcpy(message,"Searching for your new friend...");CatData found=FetchCat();Rarity rarity=RollRarity();int ci=GetRandomValue(0,4);playerCat=MakeCat(found.breed,rarity,catColours[ci]);
                    int index=-1;for(int i=0;i<collectedKinds;i++)if(strcmp(collectedBreeds[i],playerCat.breed)==0){index=i;break;}
                    bool isNew=index<0;if(isNew&&collectedKinds<MAX_CATS){index=collectedKinds++;strncpy(collectedBreeds[index],playerCat.breed,BREED_LENGTH-1);collectedBreeds[index][BREED_LENGTH-1]='\0';}
                    if(index>=0)collectedCounts[index]++;catsCollected++;
                    RarityData reward=GetRarityData(rarity);
                    if(isNew){uniqueCats++;biscuits+=reward.biscuitReward;snprintf(message,sizeof(message),"A new friend! +%d biscuits!",reward.biscuitReward);}else{biscuits++;strcpy(message,"Duplicate cat! +1 biscuit!");}hasCat=true;
                }else strcpy(message,"Oh no! You're out of fish biscuits!");
            }
        } else if(screen==BATTLE) {
            DrawText("CAT BATTLE!",300,30,32,BERRY);DrawText("Your cat",130,105,22,CUSTOM_BROWN);DrawText("Rival cat",550,105,22,CUSTOM_BROWN);
            float idleBob=sinf((float)GetTime()*4)*4;float lunge=attackAnimTime>0?sinf((1-attackAnimTime/0.42f)*PI_F)*62:0;float recoil=hitAnimTime>0?sinf((1-hitAnimTime/0.28f)*PI_F*5)*7:0;
            float playerY=245+idleBob,enemyY=245+sinf((float)GetTime()*4+1)*4;int playerX=(int)(220+lunge),enemyX=(int)(580+recoil);
            DrawEllipse(playerX,316,72,12,LIGHT_PINK);DrawEllipse(enemyX,316,72,12,LIGHT_PINK);for(int y=150;y<315;y+=24)DrawLine(400,y,400,y+10,Fade(CUSTOM_PINK,0.65f));
            DrawCat(playerX,(int)playerY,playerCat.furColour,0.9f);DrawCat(enemyX,(int)enemyY,enemyCat.furColour,0.9f);
            if(defendAnimTime>0){DrawCircleLines(playerX,(int)playerY,91+sinf((float)GetTime()*18)*4,SKYBLUE);DrawText("SHIELD!",playerX-48,(int)playerY-112,20,SKYBLUE);}
            if(attackAnimTime>0)DrawHitBurst(enemyX-20,(int)enemyY+8,attackAnimTime,specialAttack?MAGENTA:DARK_PINK);
            if(hitAnimTime>0)DrawHitBurst(playerX+20,(int)playerY+8,hitAnimTime,ORANGE);
            if(floatingTextTime>0){float rise=(1-floatingTextTime/0.85f)*34;int fx=floatingOnEnemy?enemyX:playerX;int fy=(int)(floatingOnEnemy?enemyY:playerY)-100-(int)rise;Color tc=strcmp(floatingLabel,"BLOCKED!")==0?SKYBLUE:(floatingOnEnemy?BERRY:ORANGE);DrawText(floatingLabel,fx-MeasureText(floatingLabel,24)/2,fy,24,tc);}
            DrawHealthBar(105,340,230,playerCat.health,playerCat.maxHealth);DrawHealthBar(465,340,230,enemyCat.health,enemyCat.maxHealth);
            RarityData pr=GetRarityData(playerCat.rarity),er=GetRarityData(enemyCat.rarity);DrawText(playerCat.breed,105,390,18,BERRY);DrawText(pr.name,105,414,16,pr.colour);DrawText(enemyCat.breed,465,390,18,BERRY);DrawText(er.name,465,414,16,er.colour);
            DrawRectangleRounded((Rectangle){90,445,620,55},0.15f,8,CUSTOM_WHITE);DrawText(battleMessage,110,463,18,BERRY);
            bool animating=attackAnimTime>0||hitAnimTime>0;
            bool attackClicked=!animating&&Button((Rectangle){65,520,200,55},"ATTACK",CUSTOM_PINK,DARK_PINK);
            bool specialClicked=!animating&&Button((Rectangle){300,520,200,55},"SPECIAL",PALE_PURPLE,CUSTOM_PINK);
            bool defendClicked=!animating&&Button((Rectangle){535,520,200,55},"DEFEND",CREAM,CUSTOM_PINK);
            if(animating){DrawRectangleRounded((Rectangle){65,520,200,55},0.25f,10,Fade(CUSTOM_PINK,0.55f));DrawRectangleRounded((Rectangle){300,520,200,55},0.25f,10,Fade(PALE_PURPLE,0.55f));DrawRectangleRounded((Rectangle){535,520,200,55},0.25f,10,Fade(CREAM,0.55f));DrawText("ATTACK",128,538,20,Fade(BERRY,0.5f));DrawText("SPECIAL",355,538,20,Fade(BERRY,0.5f));DrawText("DEFEND",592,538,20,Fade(BERRY,0.5f));}
            if(attackClicked||specialClicked||defendClicked){int damage=0;char ability[80]="";
                if(attackClicked){damage=playerCat.attack;strcpy(ability,"Claw swipe!");}
                else if(specialClicked){switch(playerCat.rarity){case COMMON:damage=14;strcpy(ability,"Scratch Attack!");break;case UNCOMMON:damage=18;strcpy(ability,"Double Pounce!");break;case RARE:playerCat.health=(playerCat.health+20>playerCat.maxHealth)?playerCat.maxHealth:playerCat.health+20;damage=8;strcpy(ability,"Nine Lives! +20 HP");break;case LEGENDARY:damage=25;strcpy(ability,"Royal Decree!");break;case MYTHICAL:damage=35;strcpy(ability,"CAT-ASTROPHE!");break;}specialAttack=true;}
                else {playerDefending=true;defendAnimTime=0.6f;snprintf(battleMessage,sizeof(battleMessage),"%.80s is bracing for impact!",playerCat.breed);strcpy(floatingLabel,"SHIELD UP!");floatingTextTime=0.85f;floatingOnEnemy=false;}
                if(!defendClicked){if(enemyDefending)damage=(damage+1)/2;enemyCat.health=(enemyCat.health-damage<0)?0:enemyCat.health-damage;enemyDefending=false;attackAnimTime=specialAttack?0.58f:0.42f;floatingDamage=damage;snprintf(floatingLabel,sizeof(floatingLabel),"-%d HP!",damage);floatingTextTime=0.85f;floatingOnEnemy=true;snprintf(battleMessage,sizeof(battleMessage),"%.65s %d damage!",ability,damage);specialAttack=false;}
                if(enemyCat.health<=0){playerWon=true;strcpy(resultMessage,"Victory! Your cat won the battle!");screen=RESULTS;}
                else {int enemyDamage=enemyCat.attack;if(playerDefending)enemyDamage=(enemyDamage+1)/2;playerCat.health=(playerCat.health-enemyDamage<0)?0:playerCat.health-enemyDamage;hitAnimTime=0.28f;floatingDamage=enemyDamage;if(playerDefending){strcpy(floatingLabel,"BLOCKED!");floatingOnEnemy=false;}else{snprintf(floatingLabel,sizeof(floatingLabel),"-%d HP!",enemyDamage);floatingOnEnemy=false;}floatingTextTime=0.85f;playerDefending=false;if(playerCat.health<=0){playerWon=false;strcpy(resultMessage,"Oh no! Your cat needs a nap.");screen=RESULTS;}else{size_t used=strlen(battleMessage);snprintf(battleMessage+used,sizeof(battleMessage)-used," Rival cat hits back for %d!",enemyDamage);}}
            }
        } else {
            DrawText(playerWon?"VICTORY!":"BATTLE OVER",270,100,40,BERRY);DrawCat(400,275,playerCat.furColour,1.0f);DrawText(resultMessage,400-MeasureText(resultMessage,20)/2,390,20,CUSTOM_BROWN);if(playerWon)DrawText("You earned 5 fish biscuits!",270,425,20,BERRY);
            if(Button((Rectangle){250,485,300,65},"BACK TO COLLECTION",CUSTOM_PINK,DARK_PINK)){if(playerWon)biscuits+=5;playerCat.health=playerCat.maxHealth;screen=COLLECTION;strcpy(message,playerWon?"A victory! +5 biscuits!":"Rest up and try again!");}
        }
        EndDrawing();
    }
#ifndef __EMSCRIPTEN__
    curl_global_cleanup();
#endif
    CloseWindow();return 0;
}
