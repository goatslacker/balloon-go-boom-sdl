class Balloon {
  public:
  
    // size of the balloon (make this private so we can't set it)
    int size;

    // dataset
    int dx;
    int dy;

    // visual coordinates
    int x;
    int y;

    // the type of balloon it is
    std::string type;

    // the image pointer
    SDL_Surface *img;

    // call the constructor
    Balloon();

    // functions
    int getX();
    int getY(int bot);
};

Balloon::Balloon () {
  x = 0;
  y = 0;
  dx = 0;
  dy = 0;
  type = "";
}
int Balloon::getX () {
  return ((x + 1) / size);
}

int Balloon::getY (int bot) {
  return (((bot - y) + 1) / size);
}
