#include <SFML/Graphics.hpp>//graphics,window, audio, and etc.
using namespace sf;
using namespace std;

int main(){
    //Create Window
    RenderWindow gameWindow(VideoMode({640, 480}), "Rendering Window");
    //Bg color
    Color bgColor = Color::Cyan;


    //Creating a ball
    CircleShape ball(50);
    //Colring the ball
    ball.setFillColor(Color::Blue);
    //Starting point
    ball.setPosition({300,200});

    //Movement Speed
    float speed = 5.0f;

    //Main game loop
    while(gameWindow.isOpen()){

        //Event handling loop
        while(const optional event = gameWindow.pollEvent()){
            // pollEvent() = checks for user actions (like closing the window)

            // 1. Check if the event is a Close event
            //is<...>() is about event type checking.
            if(event->is<Event::Closed>()){
                gameWindow.close();
            }

            // 2. Check if the event is a KeyPressed event
            if(const auto* keyEvent = event->getIf<Event::KeyPressed>()){
                //event->getIf<Event::KeyPressed>() → “Is this event a KeyPressed event? If yes, give me details.”
                if(keyEvent->scancode == Keyboard::Scancode::Up) ball.move({0, -speed});
                if(keyEvent->scancode == Keyboard::Scancode::Down) ball.move({0, speed});
                if(keyEvent->scancode == Keyboard::Scancode::Left) ball.move({-speed, 0});
                if(keyEvent->scancode == Keyboard::Scancode::Right) ball.move({speed, 0});
            }
        }

        //Clear the WIndow
        gameWindow.clear(bgColor);
        //dRAW/visibility of circle
        gameWindow.draw(ball);
        //display everything
        gameWindow.display();
    }

    return 0;
}