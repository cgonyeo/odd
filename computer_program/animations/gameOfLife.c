#include "../odd.h"

//A given led's neighbors are comprised of the three leds to the left and right
//4, 5 or 6 neighbors = dead due to overpopulation
//2 or 3 neighbors = lives
//0 or 1 neighbors = dead due to underpopulation
void calculateNextStepOfGame(double *in, double *out)
{    
    int range = 3; 
    for(int i = 0; i < NUM_LEDS; i++)
    {
        int neighborCount = 0;
        for(int j = i - range; j <= i + range; j++)
        {
            if(j == i)
                continue;
            int index = j;
            if(index < 0)
                index += NUM_LEDS;
            if(index > NUM_LEDS)
                index -= NUM_LEDS;
            neighborCount += in[index];
        }
       * (out + i) = neighborCount == 2 || neighborCount == 3 || neighborCount == 4;
    }
}

void printBoard(double *board)
{
    printf("game: ");
    for(int i = 0; i < NUM_LEDS; i++)
        printf("%d ", (int)board[i]);
    printf("\n");
}

void gameOfLife(double *params, double totalTime, odd_led_t *color, double *storage)
{
    int counterIndex = NUM_LEDS*2;

    double speed = *params;

    if(storage[counterIndex] == 0)
    {
        //First run, let's initialize some stuff
        *(storage + counterIndex) = totalTime + 1 / speed;
        int seed = (int)(params[1]);
        for(int i = 0; i < NUM_LEDS; i++)
        {
            *(storage + i) = (seed & i) == 0;
        }
        //printBoard(storage);
        calculateNextStepOfGame(storage, storage+NUM_LEDS);
        //printBoard(storage+NUM_LEDS);
    }

    if(totalTime > storage[counterIndex])
    {
        *(storage + counterIndex) = totalTime + 1 / speed;
        memcpy(storage, storage + NUM_LEDS, sizeof(double) * NUM_LEDS);
        calculateNextStepOfGame(storage, storage + NUM_LEDS);
        //printBoard(storage+NUM_LEDS);

        //int isDead = 0;
        //for(int i = 0; i < NUM_LEDS; i++)
        //{
        //    isDead |= storage[NUM_LEDS + i] != 0;
        //}

        //if(isDead)
        //{
        //    int chanceToRevive = rand() % 100;
        //    if(chanceToRevive == 0)
        //    {
        //        int location = rand() % NUM_LEDS;
        //        int size = rand() % 5;

        //        for(int i = location; i < location + size; i++)
        //        {
        //            *(storage + NUM_LEDS + (i % NUM_LEDS)) = 1;
        //            *(storage + counterIndex) = totalTime + 1 / speed;
        //        }
        //    }
        //}
    }

    double progress = (storage[counterIndex] - totalTime) / (1 / speed);

    for(int i = 0; i < NUM_LEDS; i++)
    {
        double strength = storage[i] * progress + storage[i + NUM_LEDS] * (1 - progress);
        setTempLED(i, 'r', color->R * strength);
        setTempLED(i, 'g', color->G * strength);
        setTempLED(i, 'b', color->B * strength);
    }
}
