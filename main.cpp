#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int population; // количество каннибалов
int volume; // объем котла

sem_t  empty; //семафор, отображающий необходимость готовки
sem_t  meat; //семафор, отображающий сколько остлось мяса

pthread_mutex_t mutexCooking; //мутекс для операции готовки
pthread_mutex_t mutexEat; //мутекс для трапезы

void errMessage1() {
    printf("%s%s%s", "incorrect command line!\n",
           "  Waited:\n",
           "     command n(number of cannibals 1 - 100) m(cauldron volume 1 - 1000)\n");
}

void errMessage2() {
    printf("%s%s%s", "incorrect qualifier value!\n",
           "  Waited:\n",
           "     command n(number of cannibals 1 - 100) m(cauldron volume 1 - 1000)\n");
}

//Повар
void *Cook(void*) {
    while (true) {
        pthread_mutex_lock(&mutexCooking); // секция приготовления мяса

        //критическая секция
        sem_wait(&empty); // ожидания пока не уведомят о том, что надо готовить
        int rnd = rand() % (volume / 2) + volume / 2;
        for (int i = 0; i < rnd; ++i) {
            sem_post(&meat); // добавление мяса в котел
        }

        pthread_mutex_unlock(&mutexCooking);

        printf("Cook: cooked %d meat\n", rnd);
    }
}

//Каннибал
void *Cannibal(void *param) {
    int cNum = *((int*)param);
    while (true) {
        pthread_mutex_lock(&mutexEat); // секция трапезы каннибала

        int isMeat;
        sem_getvalue(&meat, &isMeat);
        if(isMeat == 0) {
            sem_post(&empty) ; // необходимо готовить
        }

        sem_wait(&meat) ; // количество мяса уменьшить на единицу

        pthread_mutex_unlock(&mutexEat);

        int rnd = rand() % (population / 2) + population / 2;
        // вывод проделанной операции
        printf("Cannibal %d: ate the meat and will be full for %d s\n", cNum, rnd) ;
        sleep(rnd);
    }
}

int main(int argc, char* argv[]) {

    if(argc != 3) {
        errMessage1();
        return 1;
    }

    printf("%s", "Start\n");

    population = atoi(argv[1]); // количество канибалов
    volume = atoi(argv[2]); // объем котла

    if((population < 1) || (population > 100) || (volume < 1) || (volume > 1000)) {
        errMessage2();
        return 1;
    }

    printf("%s%d%s%d%s","Number of cannibals: ", population, ", cauldron volume: ", volume, "\n");

    // системные часы в качестве инициализатора
    srand(static_cast<unsigned int>(time(0)));

    //инициализация мутексов
    pthread_mutex_init(&mutexEat, NULL); // мутекс приема пищи
    pthread_mutex_init(&mutexCooking, NULL); // мутекс приготовления пищи
    //инициализация семафоров
    sem_init(&empty, 0, 0); //пустой ли котел
    sem_init(&meat, 0, 0); //количество кусков мяса в котле

    //запуск повара
    pthread_t threadCook;
    pthread_create(&threadCook,NULL,Cook, NULL);

    //запуск потоков-канибалов
    pthread_t threadCannibals[population];
    int cannibals[population];
    for (int i = 0; i < population ; i++) {
        cannibals[i] = i + 1;
        pthread_create(&threadCannibals[i],NULL,Cannibal, (void*)(cannibals+i));
    }

    pthread_join(threadCook, NULL);

    return 0;
}
