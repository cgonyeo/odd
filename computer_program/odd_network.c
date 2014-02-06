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

odd_led_t *parseJsonArrayToColor(json_t *colorJsonArray)
{
    char *errorCode = malloc(sizeof(char) * 64);

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
        colorTemp->R = json_integer_value(redJson);
        colorTemp->G = json_integer_value(greenJson);
        colorTemp->B = json_integer_value(blueJson);
    }
    free(errorCode);
    return color;

    objectError:
        fprintf(stderr, "error; %s is not an object\n", errorCode);
        goto error;
    integerError:
        fprintf(stderr, "error; %s is not an integer\n", errorCode);
        goto error;
    error:
        free(errorCode);
        return NULL;
}

double *parseJsonArrayToParams(json_t *paramsJson)
{
    char *errorCode = malloc(sizeof(char) * 64);

    double *params = malloc(sizeof(double) * json_array_size(paramsJson));
    for(int i = 0; i < json_array_size(paramsJson); i++)
    {
        json_t *tempJson = json_array_get(paramsJson, i);
        if(!json_is_number(tempJson)) {
            errorCode = "params\0";
            goto numberError;
        }
        params[i] = json_number_value(tempJson);
    }
    free(errorCode);
    return params;

    numberError:
        fprintf(stderr, "error: %s is not an number\n", errorCode);
        free(errorCode);
        return NULL;
}

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
        printf("New connection\n");

        pthread_t handler;
        int *num = malloc(sizeof(int));
        memcpy(num, &conn_s, sizeof(int));
        int err = pthread_create(&handler,NULL,handleConnection,num);
        if(err != 0) 
               perror("pthread_create");
    }
}

void *handleConnection(void *num)
{
    printf("Handling new connection\n");
    char buffer[1024];
    int conn_s = *((int *)num);
    free(num);
    int conn_status = read(conn_s, &buffer, 1023);
    json_t *root;
    json_error_t jsonError;
    char *errorCode = malloc(sizeof(char) * 64);
    if(conn_status <= 0)
        perror("Connection");
    buffer[conn_status] = '\0';
    while(conn_status > 0)
    {
        printf("Received %d: %s\n", conn_status, buffer);
        //Load the json
        root = json_loads(&(buffer[0]), 0, &jsonError);
        //Check for errors reading the json
        if(!root)
        {
            fprintf(stderr, "error: on line %d: %s\n", jsonError.line, jsonError.text);
            continue;
        }
        //Check that we have a json object, and not an array or some shit
        if(!json_is_object(root)) {
            errorCode = "root\0";
            goto objectError;
        }

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
            "action": "add",
            "animation": {
                "name": "cylonEye",
                "modifier": "add",
                "params": [ 0.5, 13 ],
                "colors": [ { "r": 1000, "g": 400, "b": 0 } ]
            }
        }
#endif
        if(strcmp(action, "add") == 0)
        {
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

            //Let's get the name and modifier
            const char *name = json_string_value(nameJson);
            const char *modifier = json_string_value(modifierJson);

            printf("Name: %s\n", name);

            //Parsing the parameters
            double *params = parseJsonArrayToParams(paramsJson);
            int paramCount = json_array_size(paramsJson);

            printf("params: ");
            for(int i = 0; i < paramCount; i++) {
                printf("%f,", params[i]);
            }
            printf("\n");

            //Parsing the colors
            odd_led_t *color = parseJsonArrayToColor(colorJsonArray);
            printf("Colors: r%d g%d b%d\n", color->R, color->G, color->B);

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
        //Remove a animation
        //Animation number of -1 means remove all
#if 0
        Sample JSON:
        {
            "action": "remove",
            "animation": 1
        }
#endif
        if(strcmp(action, "remove") == 0)
        {
            json_t *numJson = json_object_get(root, "animation");
            if(!json_is_integer(numJson)) {
                errorCode = "num for remove\0";
                goto integerError;
            }
            int num = json_integer_value(numJson);
            if(num == -1)
            {
                for(int i = 0; i < getNumAnimations(); i++)
                {
                    removeAnimation(0);
                }
            }
            else
                removeAnimation(num);
        }
        //List available animations
#if 0
        Sample JSON:
        {
            "action": "ls"
        }
#endif
        if(strcmp(action, "ls") == 0)
        {
            char *message = getAnimationsInJson();
            if(write(conn_s, message, strlen(message)) < 0)
            {
                printf("Error writing\n");
                exit(EXIT_FAILURE);
            }
            free(message);
        }

        //Let's read in another command
        conn_status = read(conn_s, &buffer, 1023);
        //Check for an error with the read call
        if(conn_status <= 0)
        {
            perror("Connection");
            break;
        }
        buffer[conn_status] = '\0';

        if(0)
        {
            integerError:
                fprintf(stderr, "error: %s is not an integer\n", errorCode);
                goto decref;
            stringError:
                fprintf(stderr, "error: %s is not a string\n", errorCode);
                goto decref;
            objectError:
                fprintf(stderr, "error: %s is not an object\n", errorCode);
                goto decref;
            arrayError:
                fprintf(stderr, "error: %s is not an array\n", errorCode);
                goto decref;
            decref:
                json_decref(root);
        }

    }
    printf("Closing connection\n");
	if(close(conn_s) < 0)
	{
		printf("Error closing list_s\n");
	}
    free(errorCode);
    return NULL;
}
