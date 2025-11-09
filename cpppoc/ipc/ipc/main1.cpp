#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstring>
#include <unistd.h>

struct SharedData {
    int num1;
    int num2;
    int result;
    char operation;
};

int main() {
    // Generate a unique key
    key_t key = ftok("/tmp", 65);

    // Create shared memory segment
    int shmid = shmget(key, sizeof(SharedData), 0666 | IPC_CREAT);
    if (shmid == -1) {
        std::cerr << "Shared memory creation failed" << std::endl;
        return 1;
    }

    // Attach to shared memory
    SharedData* sharedMemory = (SharedData*)shmat(shmid, nullptr, 0);
    if (sharedMemory == (void*)-1) {
        std::cerr << "Attachment failed" << std::endl;
        return 1;
    }

    // Get input from user
    std::cout << "Enter first number: ";
    std::cin >> sharedMemory->num1;

    std::cout << "Enter second number: ";
    std::cin >> sharedMemory->num2;

    std::cout << "Enter operation (+, -, *, /): ";
    std::cin >> sharedMemory->operation;

    // Detach from shared memory
    shmdt(sharedMemory);

    // Launch second process
    pid_t pid = fork();
    if (pid == 0) {
        // Child process - execute main2
        execl("./main2", "main2", nullptr);
        exit(1);
    }

    return 0;
}
