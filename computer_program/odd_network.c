#include "odd.h"

#define ANIMATION(animationName, params, num1, num2) {\
    .name=#animationName,\
    .function=animationName,\
    .paramDescriptions=#params,\
    .numParams=num1,\
    .numColors=num2\
},
Animation animation_list[] = {
#include "animations.def"
};
#undef ANIMATION

int animation_list_c;


void *networkListen(char *buffer)
{

animation_list_c = 0;
#define ANIMATION(a, b, c, d) animation_list_c++;
#include "animations.def"
#undef ANIMATION

    int list_s;        //Listening socket
    int conn_s;        //connection socket
    short int port = INCPORT;    //port number
    struct sockaddr_in servaddr;    //socket address struct
    printf("Starting socket\n");
    if((list_s = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        printf("Error making listening socket\n");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    servaddr.sin_port = htons(port);

    int yes = 1;

    if(setsockopt(list_s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        printf("Error setting socket options\n");
        exit(EXIT_FAILURE);
    }

    printf("Binding to socket\n");
    if(bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        printf("Error calling bind\n");
        exit(EXIT_FAILURE);
    }
    printf("Setting socket to listen\n");
    if(listen(list_s, LISTENQ) < 0)
    {
        printf("Error calling listen\n");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        printf("Accepting on socket\n");
        if((conn_s = accept(list_s, NULL, NULL)) < 0)
        {
            printf("Error calling accept\n");
            exit(EXIT_FAILURE);
        }

        printf("Connection made\n");
        pthread_t handler;
        int *num = malloc(sizeof(int));
        memcpy(num, &conn_s, sizeof(int));
        pthread_create(&handler,NULL,handleConnection,num);
    }
}

void *handleConnection(void *num)
{
    char buffer[256];
    int conn_s = *((int *)num);
    free(num);
    int conn_status = read(conn_s, &buffer, 255);
    json_t *root;
    json_error_t jsonError;
    char *errorCode = malloc(sizeof(char) * 64);
    if(conn_status <= 0)
        perror("Connection");
    while(conn_status > 0)
    {
        printf("Received %d: %s\n", conn_status, buffer);
        conn_status = read(conn_s, &buffer, 255);
        //Check for an error with the read call
        if(conn_status < 0)
        {
            perror("Connection");
            break;
        }
        printf("1\n");
        //Load the json
        root = json_loads(&(buffer[0]), 0, &jsonError);
        //Check for errors reading the json
        if(!root)
        {
            fprintf(stderr, "error: on line %d: %s\n", jsonError.line, jsonError.text);
            break;
        }
        printf("2\n");
        //Check that we have a json object, and not an array or some shit
        if(!json_is_object(root)) {
            errorCode = "root\0";
            goto objectError;
        }

        printf("3\n");
        //The type of command is stored as a string under "action"
        json_t *actionJson = json_object_get(root, "action");
        if(!json_is_string(actionJson)) {
            errorCode = "action\0";
            goto stringError;
        }

        const char *action = json_string_value(actionJson);
        printf("action: %s\n", action);
        //Add an animation
#if 0
        Sample JSON:
        {
            "action": "add"
            "name": "cylonEye"
            "modifier": "add"
            "params": [ 0.5, 13 ]
            "colors": [ { "r": 1000 "g": 400 "b": 0 } ]
        }
#endif
        if(strcmp(action, "add") == 0)
        {
            printf("We're adding\n");
            json_t *animationJson = json_object_get(root, "animation");
            if(!json_is_object(animationJson)) {
                errorCode = "animation\0";
                goto objectError;
            }

            //Let's get the various parameters
            json_t *nameJson = json_object_get(animationJson, "name"); //String
            json_t *modifierJson = json_object_get(animationJson, "modifier"); //String
            json_t *paramsJson = json_object_get(animationJson, "params"); //Array
            json_t *colorJsonArray = json_object_get(animationJson, "colors"); //Array
            //Error checking
            if(!json_is_string(nameJson)) {
                errorCode = "name\0";
                goto stringError;
            }
            if(!json_is_string(modifierJson)) {
                errorCode = "modifier\0";
                goto stringError;
            }
            if(!json_is_array(paramsJson)) {
                errorCode = "params\0";
                goto arrayError;
            }
            if(!json_is_array(colorJsonArray)) {
                errorCode = "colorJsonArray\0";
                goto arrayError;
            }

            const char *name = json_string_value(nameJson);
            const char *modifier = json_string_value(modifierJson);

            printf("Name: %s\n", name);

            //Parsing the parameters
            double *params = malloc(sizeof(double) * json_array_size(paramsJson));
            int paramCount = json_array_size(paramsJson);
            for(int i = 0; i < json_array_size(paramsJson); i++)
            {
                json_t *tempJson = json_array_get(paramsJson, i);
                if(!json_is_number(tempJson)) {
                    errorCode = "params\0";
                    goto numberError;
                }
                params[i] = json_number_value(tempJson);
            }

            printf("params: ");
            for(int i = 0; i < paramCount; i++) {
                printf("%f,", params[i]);
            }
            printf("\n");

            //Parsing the colors
            odd_led_t *color = NULL;
            odd_led_t *colorTemp = NULL;

            for(int i = 0; i < json_array_size(colorJsonArray); i++)
            {
                if(colorTemp == NULL)
                {
                    color = malloc(sizeof(odd_led_t));
                    colorTemp = color;
                    colorTemp->next = NULL;
                }
                else
                {
                    colorTemp->next = malloc(sizeof(odd_led_t));
                    colorTemp = colorTemp->next;
                    colorTemp->next = NULL;
                }
                //Get the object out of the array
                json_t *colorJson = json_array_get(colorJsonArray, i);
                if(!json_is_object(colorJson)) {
                    errorCode = "colorJson\0";
                    goto objectError;
                }
                //Get the colors out of the object
                json_t *redJson =   json_object_get(colorJson, "r");
                json_t *greenJson = json_object_get(colorJson, "g");
                json_t *blueJson =  json_object_get(colorJson, "b");
                if(!json_is_integer(redJson)) {
                    errorCode = "redJson";
                    goto integerError;
                }
                if(!json_is_integer(greenJson)) {
                    errorCode = "greenJson";
                    goto integerError;
                }
                if(!json_is_integer(blueJson)) {
                    errorCode = "blueJson";
                    goto integerError;
                }
                //Save the colors into the struct
                printf("Colors: r:%d g:%d b:%d\n", json_integer_value(redJson), json_integer_value(greenJson), json_integer_value(blueJson));
                colorTemp->R = json_integer_value(redJson);
                colorTemp->G = json_integer_value(greenJson);
                colorTemp->B = json_integer_value(blueJson);
            }

            //Add the animation
            for(int i = 0; i < animation_list_c; i++)
            {
                if(strcmp(name, animation_list[i].name) == 0)
                {
                    printf("Adding animation for playback\n");
                    addAnimation(animation_list[i].function, params, color, addLeds);
                }
            }
        }
        break;

        stringError:
            fprintf(stderr, "error: %s is not a string\n", errorCode);
            goto decref;
        objectError:
            fprintf(stderr, "error: %s is not an object\n", errorCode);
            goto decref;
        arrayError:
            fprintf(stderr, "error: %s is not an array\n", errorCode);
            goto decref;
        numberError:
            fprintf(stderr, "error: %s is not an number\n", errorCode);
            goto decref;
        integerError:
            fprintf(stderr, "error: %s is not an integer\n", errorCode);
            goto decref;
        decref:
            json_decref(root);
    }
    free(errorCode);
    return NULL;
}
