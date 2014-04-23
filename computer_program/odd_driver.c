#include "odd.h"

#define ANIMATION(func, animationName, params, num1, num2, num3) {\
    .name=#animationName,\
    .function=func,\
    .paramDescriptions=#params,\
    .numParams=num1,\
    .numColors=num2,\
    .storageSize=num3\
},
Animation animation_list2[] = {
#include "animations.def"
};
#undef ANIMATION
int animation_list_c2 = 0;

long double totalTime, elapsedTime;
int done = 0;
int numAnimations = 0;
int timeLoops = 0;

pthread_mutex_t ledsLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ledsTempLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t animationListLock = PTHREAD_MUTEX_INITIALIZER;

odd_led_t* leds[NUM_LEDS]; //All the LEDs in use
odd_led_t* tempLeds[NUM_LEDS]; //Current alterations to the LEDs, used with animations
animation_t* animations = NULL; //All currently used animations.

//Returns the value of the color of led i in a thread-safe manner
int getLED(int i, char color)
{
    int returnValue;
    pthread_mutex_lock(&ledsLock);
    switch(color)
    {
        case 'R': case 'r': returnValue = leds[i]->R; break;
        case 'G': case 'g': returnValue = leds[i]->G; break;
        case 'B': case 'b': returnValue = leds[i]->B; break;
    }
    pthread_mutex_unlock(&ledsLock);
    return returnValue;
}

//Returns the value of the color of templed i in a thread-safe manner
int getTempLED(int i, char color)
{
    int returnValue;
    pthread_mutex_lock(&ledsTempLock);
    switch(color)
    {
        case 'R': case 'r': returnValue = tempLeds[i]->R; break;
        case 'G': case 'g': returnValue = tempLeds[i]->G; break;
        case 'B': case 'b': returnValue = tempLeds[i]->B; break;
    }
    pthread_mutex_unlock(&ledsTempLock);
    return returnValue;
}

//sets the color of led i to value in a thread-safe manner
void setLED(int i, char color, int value)
{
    pthread_mutex_lock(&ledsLock);
    switch(color)
    {
        case 'R': case 'r': leds[i]->R = value; break;
        case 'G': case 'g': leds[i]->G = value; break;
        case 'B': case 'b': leds[i]->B = value; break;
    }
    pthread_mutex_unlock(&ledsLock);
}

//sets the color of templed i to value in a thread-safe manner
void setTempLED(int i, char color, int value)
{
    pthread_mutex_lock(&ledsTempLock);
    switch(color)
    {
        case 'R': case 'r': tempLeds[i]->R = value; break;
        case 'G': case 'g': tempLeds[i]->G = value; break;
        case 'B': case 'b': tempLeds[i]->B = value; break;
    }
    pthread_mutex_unlock(&ledsTempLock);
}

//Writes the led array to the console
void write_console(void) {
    printf("\n");
    for(int i=0; i<NUM_LEDS; i++) {
        printf("%d, ", getLED(i, 'r'));
        fflush(NULL);
    }
}

//Sends the new LED values to the hardware
void write_odd(void) {
    for(int j = 0; j < NUM_TLCS; j++)
        for(int i = 0; i < 8; i++)
        {
            tlcSetLed(j * 24 + i*3,   getLED(j * 8 + i, 'r'));
            tlcSetLed(j * 24 + i*3+1, getLED(j * 8 + i, 'g'));
            tlcSetLed(j * 24 + i*3+2, getLED(j * 8 + i, 'b'));
        }
    tlcUpdateLeds();
}

//Resets all LEDs to 0
void resetLeds(void)
{
    for(int i = 0; i < NUM_LEDS; i++)
    {
        setLED(i, 'r', 0);
        setLED(i, 'g', 0);
        setLED(i, 'b', 0);
    }
}

//Adds an animation
void addAnimation( animation_f function, double* params, odd_led_t* color, modifier_f modifier)
{
    pthread_mutex_lock(&animationListLock);
    animation_t* newAnimation;
    if ((newAnimation = malloc(sizeof(animation_t))) == NULL) {
        fprintf(stderr, "Malloc failed");
        exit(1);
    }
    newAnimation->function = function;
    newAnimation->params = params;
    newAnimation->color = color;
    newAnimation->modifier = modifier;
    newAnimation->next = NULL;

    for(int i = 0; i < animation_list_c2; i++)
    {
        if(animation_list2[i].function == function)
        {
            int size = sizeof(double) * animation_list2[i].storageSize;
            newAnimation->storage = malloc(size);
            memset(newAnimation->storage, 0, size);
        }
    }

    if(numAnimations == 0)
        animations = newAnimation;
    else {
        animation_t *temp = animations;
        while(temp->next != NULL)
            temp = temp->next;
        temp->next = newAnimation;
    }
    numAnimations++;
    pthread_mutex_unlock(&animationListLock);
}

//Updates an animation
void updateAnimation( int index, double* params, odd_led_t* color)
{
    pthread_mutex_lock(&animationListLock);
    int counter = 0;
    animation_t *temp = animations;
    while(counter < index) {
        temp = temp->next;
        counter++;
    }
    temp->params = params;
    odd_led_t *tempColor = temp->color;
    temp->color = color;
    free(tempColor);
    pthread_mutex_unlock(&animationListLock);
}

//Removes an animation
void removeAnimation(int index)
{
    pthread_mutex_lock(&animationListLock);
    if(index >= numAnimations || index < 0)
    {
        pthread_mutex_unlock(&animationListLock);
        return;
    }
    if(index == 0)
        animations = animations->next;
    else
    {
        int i = 0;
        animation_t *temp = animations;
        while(i < index - 1)
        {
            temp = temp->next;
            i++;
        }
        free(temp->next->params);
        free(temp->next->storage);
        free(temp->next->color);
        free(temp->next);
        temp->next = temp->next->next;
    }
    numAnimations--;
    pthread_mutex_unlock(&animationListLock);
}

int getNumAnimations()
{
    return numAnimations;
}

//gets the current animations in json form
char *getAnimationsInJson()
{
    pthread_mutex_lock(&animationListLock);
    json_t *animArray = json_array();
    printf("Current animation count: %d\n", numAnimations);
    animation_t *anim = animations;
    for(int i = 0; i < numAnimations; i++)
    {
        json_t *animObject = json_object();
        for(int j = 0; j < animation_list_c2; j++)
        {
            if(animation_list2[j].function == anim->function)
            {
                json_t *name = json_pack("s",animation_list2[j].name);
                json_object_set(animObject, "name", name);

                int numParams = animation_list2[j].numParams;
                json_t *paramsArray = json_array();
                for(int k = 0; k < numParams; k++)
                    json_array_append(paramsArray, json_pack("f", anim->params[k]));
                json_object_set(animObject, "params", paramsArray);

            }
        }

        json_t *colorArray = json_array();
        odd_led_t *colorTemp = anim->color;
        while(colorTemp != NULL)
        {
            json_t *colorJsonObject = json_object();
            json_object_set(colorJsonObject, "r", json_pack("i", colorTemp->R));
            json_object_set(colorJsonObject, "g", json_pack("i", colorTemp->G));
            json_object_set(colorJsonObject, "b", json_pack("i", colorTemp->B));
            json_array_append(colorArray, colorJsonObject);
            colorTemp = colorTemp->next;
        }
        json_object_set(animObject, "colors", colorArray);

        json_t *modifier = json_pack("s", "add");
        json_object_set(animObject, "modifier", modifier);
        json_array_append(animArray, animObject);

        anim = anim->next;
    }
    pthread_mutex_unlock(&animationListLock);
    char *returnValue = json_dumps(animArray, 0);
    json_decref(animArray);
    return returnValue;
}

//Program's update loop
void *updateLoop(void *arg) {
    (void)arg;
    int failed = 0;
    srand(time(NULL));
    
    elapsedTime = 0;
    totalTime = 0;
    struct timeval start, current, previous;
    gettimeofday(&start, NULL);
    gettimeofday(&previous, NULL);

    while(!done)
    {
        resetLeds();
        previous = current;
        gettimeofday(&current, NULL);
        totalTime = formatTime(current.tv_sec, current.tv_usec);// - formatTime(start.tv_sec, start.tv_usec);

        animation_t *anim = animations;
        for(int i = 0; i < numAnimations; i++)
        {
            anim->function(anim->params, totalTime, anim->color, anim->storage);
            anim->modifier(leds, tempLeds);

            anim = anim->next;
        }
                
        if(failed==0)
            write_odd();

        //write_console();
        usleep(500);
    }
    resetLeds();
    if(failed==0)
        write_odd();
    return NULL;
}

static void *none(void *arg) {
    return NULL;
}

void thread_test() {
    pthread_t thd[1000];
    pthread_attr_t attrs;
    pthread_attr_init(&attrs);
    //pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    int i = 0;
    while (pthread_create(thd+i, &attrs, none, NULL) == 0) i++;
    printf("I can run %d threads\n", i);
    pthread_attr_destroy(&attrs);
    for (int j = 0; j < i; j++) pthread_join(thd[j], NULL);
}


int main(void)
{

    animation_list_c2 = 0;
    #define ANIMATION(a, b, c, d, e, f) animation_list_c2++;
    #include "animations.def"
    #undef ANIMATION

    for(int i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = malloc(sizeof(odd_led_t));
        tempLeds[i] = malloc(sizeof(odd_led_t));
    }

    tlc5947init();

    audioInitialization();
    thread_test();
    //Start the thread that updates our LEDs
    pthread_t ul;
    pthread_create(&ul,NULL,updateLoop,"randomargs");

    printf("ODD started.\n");
    
    //odd_led_t* color = malloc(sizeof(odd_led_t));
    //color->R = 0;
    //color->G = 4095;
    //color->B = 0;
    //color->next = NULL;
    //
    //double params[2];
    //params[0] = 5;
    //params[1] = 20;

    //addAnimation(gameOfLife, params, color, addLeds);

    //odd_led_t* color2 = malloc(sizeof(odd_led_t));
    //color2->R = 0;
    //color2->G = 0;
    //color2->B = 4095;
    //
    //double params2[2];
    //params2[0] = 0.45;
    //params2[1] = 24;

    //addAnimation(cylonEye, params2, color2, addLeds);

    //odd_led_t* color3 = malloc(sizeof(odd_led_t));
    //color3->R = 4095;
    //color3->G = 0;
    //color3->B = 0;
    //
    //double params3[2];
    //params3[0] = 0.4;
    //params3[1] = 24;

    //addAnimation(cylonEye, params3, color3, addLeds);

    //odd_led_t* color4 = malloc(sizeof(odd_led_t));
    //color4->R = 4095;
    //color4->G = 4095;
    //color4->B = 4095;
    //
    //double params4[2];
    //params4[0] = 30;
    //params4[1] = 24;

    //addAnimation(strobe, params4, color4, subtractLeds);

    networkListen();
    printf("Exiting...\n");
    done = 1;
    pthread_join(ul, NULL);
    tlc5947cleanup();
    for(int i = 0; i < numAnimations; i++)
    {
        //free(animations[numAnimations]->color);
        animation_t *temp = animations;
        animations = animations->next;
        free(temp);
    }
    //for(int i = 0; i < NUM_LEDS; i++)
    //{
    //    free(leds[i]);
    //    free(tempLeds[i]);
    //}
}

