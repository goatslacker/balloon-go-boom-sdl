#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include <string>
#include <sstream>
#include "timer.h"
#include "balloon.h"
//#include "cloud.h"
#include <vector>

// TODO
/*
  -gravity?
  -rotation (or column splicing instead...)
  -gameover
*/

using namespace std;

// debug
#include <iostream>

const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 24; // 32
const int FRAMES_PER_SECOND = 20;

SDL_Event event;

// pointers
SDL_Surface *screen = NULL;
SDL_Surface *meadow = NULL;
SDL_Surface *sky = NULL;

// see if we can fit this all into one object
//SDL_Surface *clouds[4];
//Cloud cloudPos[4];

// BalloonGoBoom variables
int bgb_square = 8;
int bgb_size = 40;
int bgb_score = 0;
string bgb_balloons[4] = { "blue", "yellow", "purple", "red" };
string bgb_balloons_dead[2] = { "black", "boom" };
vector<SDL_Rect> bgb_found;

// multi-dim vector
vector< vector<Balloon> > bgb_grid(bgb_square, vector<Balloon>(bgb_square));

// the bottom of the screen (align bottom)
const int bgb_bottom = SCREEN_HEIGHT - bgb_size;

// SDL Functions
SDL_Surface *load_image (string filename) {
  SDL_Surface* loadedImage = NULL;
  SDL_Surface* optimizedImage = NULL;

  // Load the image
  loadedImage = IMG_Load(filename.c_str());

  // If nothing went wrong in loading the image
  if (loadedImage != NULL) {
    // Create an optimized image
    optimizedImage = SDL_DisplayFormatAlpha(loadedImage);

    // If the image was optimized just fine
/*
    if (optimizedImage != NULL) {
      Uint32 colorkey = SDL_MapRGB(optimizedImage->format, 0xFF, 0xFF, 0xFF);
      SDL_SetColorKey(optimizedImage, SDL_SRCCOLORKEY, colorkey);
    }
*/

    // Free the old image
    SDL_FreeSurface(loadedImage);
  }

  //Return the optimized image
  return optimizedImage;
}

void clean_up () {
  bgb_grid.clear();
  bgb_found.clear();

  SDL_FreeSurface(sky);
  SDL_FreeSurface(meadow);

  // SDL_FreeSurface all the balloons

  SDL_Quit();
}

void apply_surface (int x, int y, SDL_Surface* source, SDL_Surface* destination) {
  //Make a temporary rectangle to hold the offsets
  SDL_Rect offset;

  //Give the offsets to the rectangle
  offset.x = x;
  offset.y = y;

  //Blit the surface
  SDL_BlitSurface(source, NULL, destination, &offset);
}

void updateSky () {
  int i = 0;

  for (i = 0; i < SCREEN_WIDTH; i++) {
    apply_surface(i, 0, sky, screen);
  }
}

void updateMeadow () {
  apply_surface(((meadow->w / 2) - SCREEN_WIDTH), (SCREEN_HEIGHT - meadow->h), meadow, screen);
}

bool found_in_array(SDL_Rect obj) {
  int i = 0;

  for (i = 0; i < bgb_found.size(); i++) {
    if (bgb_found[i].x == obj.x && bgb_found[i].y == obj.y) {
      return true;
    }
  }

  return false;
}




// BalloonGoBoom Functions

void BalloonGoBoom_reset () {
  // blank the screen
  updateSky();
  updateMeadow();
  SDL_Flip(screen);

  bgb_score = 0;
  bgb_found.clear();
}

SDL_Surface *BalloonGoBoom_blit (Balloon balloon) {
  SDL_Surface* img;
  stringstream balloon_image;

  balloon_image << "images/" << balloon.type <<".png";

  // load the image
  img = load_image(balloon_image.str());

  // TODO shouldn't always have to load the image... lets see if i can just apply it without loading it so many times

  // blit
  apply_surface(balloon.x, balloon.y, img, screen);

  return img;
}

void BalloonGoBoom_draw () {
  int y = 0;
  int x = 0;

  for (x = 0; x < bgb_square; x++) {
    for (y = 0; y < bgb_grid[x].size(); y++) {
      bgb_grid[x][y].img = BalloonGoBoom_blit(bgb_grid[x][y]);
    }
  }
}

void BalloonGoBoom_pointers () { // restruct pointers
  int x = 0;
  int y = 0;
  vector< vector<Balloon> > tmpgrid(bgb_square, vector<Balloon>());
  Balloon balloon;


  // tmpgrid is not loading right...
  
  // load into tmp var
  for (x = 0; x < bgb_square; x++) {
    for (y = 0; y < bgb_grid[x].size(); y++) {

      if (bgb_grid[x][y].type != "null") {
        // reset size
        balloon.size = bgb_grid[x][y].size;

        // dataset xy
        balloon.dx = x;
        balloon.dy = tmpgrid[x].size();

        // visual xy
        balloon.x = x * bgb_size;
        balloon.y = bgb_bottom - ((tmpgrid[x].size()) * bgb_size);

        // balloon type and image
        balloon.type = bgb_grid[x][y].type;
        balloon.img = bgb_grid[x][y].img;

        // add to pointer
        tmpgrid[x].push_back(balloon);
      }
    }
  }

  // clears everything!
  //bgb_grid.clear();

  // just clears the second dimension
  for (x = 0; x < bgb_square; x++) {
    bgb_grid[x].clear();
  }


  // set the pointer
  for (x = 0; x < bgb_square; x++) {
    // erase the old data
    //bgb_grid[x].erase(bgb_grid[x].begin() + bgb_grid[x].size());
    for (y = 0; y < tmpgrid[x].size(); y++) {

      bgb_grid[x].push_back(tmpgrid[x][y]);

      cout << "x: " << x << ", y: " << bgb_grid[x].size() - 1;
      cout << "; ";
      cout << "xpos: " << bgb_grid[x][bgb_grid[x].size() - 1].x << ", ypos: " << bgb_grid[x][bgb_grid[x].size() - 1].y;
      cout << "; ";
      cout << "type: " << bgb_grid[x][bgb_grid[x].size() - 1].type;
      cout << "\n";

    }
    cout << "\n";
  }

  // dont think i need to do this:

  // update the screen
  updateSky();
  updateMeadow();
  BalloonGoBoom_draw();

  SDL_Flip(screen);
}

void BalloonGoBoom_gravity () {

  BalloonGoBoom_pointers();



/*
  int y = 0;
  int x = 0;

  //int ground = 0;

  // tmp objs
  SDL_Rect obj;
  int pullr;

  // arrays
  vector<Balloon> drop;
  vector<int> pull(bgb_size, 0);

  // find all objects we need to move
  for (x = 0; x < bgb_square; x++) {
    pullr = 0;

    for (y = 0; y < bgb_square; y++) {
      if (bgb_grid[x][y].type == "null") {
        pullr++;
      } else if (pullr > 0) {
        drop.push_back(bgb_grid[x][y]);
        pull[x] = pullr;
      }

    }
  }

  for (x = 0; x < drop.size(); x++) {

    // set the items to drop
    bgb_grid[drop[x].dx][drop[x].dy].y = bgb_bottom - ((drop[x].dy - pull[drop[x].dx]) * bgb_size);
    //bgb_grid[drop[x].getX()][drop[x].getY(bgb_bottom)].y = bgb_bottom - ((drop[x].getY(bgb_bottom) - pull[drop[x].getX()]) * bgb_size);


    //cout << drop[x].getX();
    //cout << ",";
    //cout << drop[x].getY(bgb_bottom);
    //cout << "\n";
    // cout << "current dataset x: " << drop[x].dx;
    //cout << "current dataset y: " << drop[x].dy;
    // current x: drop[x].x
    // current y: drop[x].y
    // pull: pull[drop[x].dx];
    // goal: (drop[x].dy - pull[drop[x].dx]) * bgb_size;

  }

  // animate here
  // use a gravity timer and wait until all elements are grounded

  // no animation
  updateSky();
  updateMeadow();
  BalloonGoBoom_draw();
  
  SDL_Flip(screen);

  drop.clear();
  
  // reset the pointers
  BalloonGoBoom_pointers();
*/
}

void BalloonGoBoom_boom () {
  int i = 0;

  updateSky();
  updateMeadow();

  for (i = 0; i < bgb_found.size(); i++) {
    bgb_grid[bgb_found[i].x][bgb_found[i].y].type = "boom";
  }

  BalloonGoBoom_draw();

  SDL_Flip(screen);

  SDL_Delay(40);

  updateSky();
  updateMeadow();
 
  for (i = 0; i < bgb_found.size(); i++) {
    bgb_grid[bgb_found[i].x][bgb_found[i].y].type = "null";
  }

  BalloonGoBoom_draw();

  SDL_Flip(screen);

  // add score
  bgb_score = bgb_score + ((bgb_size ^ 2) * 4);
  
  // erase all found elements
  bgb_found.clear();

  // apply gravity
  BalloonGoBoom_gravity();
}

void BalloonGoBoom_finder (int x, int y) {
  SDL_Rect obj;
  int i = 0;
  bool found = false;

  // dont overflow the array
  if (x + 1 < bgb_square) {

    // if we have a match
    if (bgb_grid[x][y].type == bgb_grid[x + 1][y].type) {
      obj.x = x + 1;
      obj.y = y;

      found = found_in_array(obj);

      // push into array if it's not already in the array
      if (found == false) {
        bgb_found.push_back(obj);
      }
    }
  }

  if (y + 1 < bgb_grid[x].size()) {
    if (bgb_grid[x][y].type == bgb_grid[x][y + 1].type) {
      obj.x = x;
      obj.y = y + 1;

      found = found_in_array(obj);

      if (found == false) {
        bgb_found.push_back(obj);
      }
    }
  }

  if (x - 1 >= 0) {
    if (bgb_grid[x][y].type == bgb_grid[x - 1][y].type) {
      obj.x = x - 1;
      obj.y = y;

      found = found_in_array(obj);

      if (found == false) {
        bgb_found.push_back(obj);
      }
    }
  }

  if (y - 1 >= 0) {
    if (bgb_grid[x][y].type == bgb_grid[x][y - 1].type) {
      obj.x = x;
      obj.y = y - 1;

      found = found_in_array(obj);

      if (found == false) {
        bgb_found.push_back(obj);
      }
    }
  }
  
}

void BalloonGoBoom_find (int x, int y) {
  int i = 0;
  SDL_Surface* img;

/* // debug which item is selected
  cout << x;
  cout << "\n";
  cout << y;
  cout << "\n";
  cout << "\n";
*/

  BalloonGoBoom_finder(x, y);

  while (i < bgb_found.size()) {
    BalloonGoBoom_finder(bgb_found[i].x, bgb_found[i].y);
    i++;
  }

/* // debug all found items
  for (i = 0; i < bgb_found.size(); i++) {
    cout << bgb_found[i].x;
    cout << "\n";
    cout << bgb_found[i].y;
    cout << "\n";
    cout << "\n";
  }
*/

  BalloonGoBoom_boom();
}

void BalloonGoBoom_newGame () {
  BalloonGoBoom_reset();

  int y = 0;
  int x = 0;
  int b = 0;
  // set balloon object
  Balloon balloon;

  for (x = 0; x < bgb_square; x++) {
    for (y = 0; y < bgb_square; y++) {

      // randomly select a balloon
      b = (rand() % 4 + 1) - 1;

      // set the balloon size
      balloon.size = bgb_size;

      // dataset coordinates
      balloon.dx = x;
      balloon.dy = y;

      // visual coordinates
      balloon.x = x * bgb_size;
      balloon.y = bgb_bottom - (y * bgb_size);

      // balloon type and image
      balloon.type = bgb_balloons[b];
      balloon.img = BalloonGoBoom_blit(balloon);

      // add to pointer
      bgb_grid[x][y] = balloon;
    }
  }

  SDL_Flip(screen);
}



// main program

int main (int argc, char* args[]) {
  int frame = 0;
  Timer fps;
  int i;
  int x;
  int y;
  bool quit = false;

  // set the seed
  srand(SDL_GetTicks());

  // Initialize SDL
  if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
    return 1;
  }

  // screen setting
  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
  if (screen == NULL) {
    return 1;
  }

  SDL_WM_SetCaption("Balloon Go Boom!", NULL);

  // load the images
  meadow = load_image("images/meadow-day.png");
  if (meadow == NULL) {
    return 1;
  }

  sky = load_image("images/background-day.png");
  if (sky == NULL) {
    return 1;
  }

/*
  for (i = 0; i < 4; i++) {
    generateCloud(i);
  }
*/

  updateSky();
  updateMeadow();

  SDL_Flip(screen);

  BalloonGoBoom_newGame();

  // create cloud movement
  //Timer cloudTimer;
  //cloudTimer.start();

  // cap frames/sec
  fps.start();

  while (quit == false) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        quit = true;
        break;

      case SDL_MOUSEBUTTONUP:
        for (x = 0; x < bgb_square; x++) {
          for (y = 0; y < bgb_grid[x].size(); y++) {

/*
            // need to give each balloon an x and a y
            if (x == 1 && y == 1) {
  
              cout << event.motion.x;
              cout << "\n";

              cout << x * bgb_size;
              cout << "\n";

              cout << (x * bgb_size) + bgb_size;
              cout << "\n";
              cout << "\n";

              cout << event.motion.y;
              cout << "\n";

              cout << bgb_bottom - (y * bgb_size);
              cout << "\n";

              cout << (bgb_bottom + bgb_size) - (y * bgb_size);
              cout << "\n";
              cout << "\n";
              cout << "\n";
            }
*/

            if (
              event.motion.x > x * bgb_size && 
              event.motion.x < (x * bgb_size) + bgb_size &&
              event.motion.y > bgb_bottom - (y * bgb_size) && 
              event.motion.y < (bgb_bottom + bgb_size) - (y * bgb_size)
            ) {

              if (bgb_grid[x][y].type != "null") {
                BalloonGoBoom_find(x, y);

              }

            }
          }
        }
        break;
      }
    }
   
    // timer
/*
    if (cloudTimer.get_ticks() >= 60) {
      cloudTimer.stop();
      //moveClouds();
      //cloudTimer.start();
    }
*/

    frame++;

    if (fps.get_ticks() < 1000 / FRAMES_PER_SECOND) {
      SDL_Delay((1000 / FRAMES_PER_SECOND) - fps.get_ticks());
    }
  }
  
  clean_up();
  return 0;
}
