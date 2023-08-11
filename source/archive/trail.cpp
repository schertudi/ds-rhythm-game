
#include "effects.cpp"

struct TouchEntry {
    int x;
    int y;
    int currentFrame;
    bool active;
};

class TouchTracker {

    private:
    static constexpr int MAX_POINTS = 100; // Maximum number of entries
    TouchEntry entries[MAX_POINTS];
    int currentEntry = 0;
    CircleEffect trailEffect;

    public:
    TouchTracker(int x) : trailEffect(800, 10) {
    }

    void logTouch(int x, int y) {
        entries[currentEntry].x = x;
        entries[currentEntry].y = y;
        entries[currentEntry].currentFrame = 0;
        entries[currentEntry].active = true;
        currentEntry++;
        if(currentEntry >= MAX_POINTS) {
            currentEntry = 0;
        }
    }

    void deleteOldEntries() { //delete entry
        for (int i = 0; i < MAX_POINTS; i++) {
            if(entries[i].active) {
                entries[i].currentFrame++;
                if(entries[i].currentFrame > 10) {
                    entries[i].active = false;
                }
            }
        }
    }

    void drawTrail(int frame) {
        for (int i = 0; i < MAX_POINTS; i++) {
            if(entries[i].active) {
                trailEffect.basicCircle(entries[i].x, entries[i].y, frame);
            }
        }
    }
};