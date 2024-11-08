#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#include <sys/sysctl.h>  // For macOS
#include <sys/sysinfo.h>  // For Linux
#endif

long long PerformIntegerCalculations(long long count) {
    long long result = 0;
    for (long long i = 0; i < count; i++) {
        result += i * (1 + 2 + 3 + 4); // Integer operations
    }
    return result;
}

double PerformFloatCalculations(long long count) {
    double result = 0.0;
    for (long long i = 0; i < count; i++) {
        result += i * 10; // Floating-point operations
    }
    return result;
}

long long SingleThreadedTest(long long count, int logicalProcessorCount) {
    clock_t start = clock();

    long long resultInt = 0;
    for (int i = 0; i < logicalProcessorCount / 2; i++) {
        resultInt = PerformIntegerCalculations(count);
    }

    double resultDouble = 0.0;
    for (int i = 0; i < logicalProcessorCount / 2; i++) {
        resultDouble = PerformFloatCalculations(count);
    }

    clock_t end = clock();
    long elapsed = (end - start) * 1000 / CLOCKS_PER_SEC;

    double score = ((double)count * logicalProcessorCount) / elapsed / 10000;
    printf("SingleThread Score: %.2f\n", score);
    return score;
}

#ifdef _WIN32
DWORD WINAPI MultiThreadedIntegerCalculations(LPVOID param) {
    long long count = *((long long*)param);
    return PerformIntegerCalculations(count);
}

DWORD WINAPI MultiThreadedFloatCalculations(LPVOID param) {
    long long count = *((long long*)param);
    return PerformFloatCalculations(count);
}

long long MultiThreadedTest(long long count, int logicalProcessorCount) {
    clock_t start = clock();

    HANDLE* threads = (HANDLE*)malloc(sizeof(HANDLE) * logicalProcessorCount);
    DWORD* threadResults = (DWORD*)malloc(sizeof(DWORD) * logicalProcessorCount);

    long long resultInt = 0;
    double resultDouble = 0.0;

    // Launch threads for integer and floating point calculations
    for (int i = 0; i < logicalProcessorCount / 2; i++) {
        threads[i] = CreateThread(NULL, 0, MultiThreadedIntegerCalculations, (LPVOID)&count, 0, &threadResults[i]);
        threads[i + logicalProcessorCount / 2] = CreateThread(NULL, 0, MultiThreadedFloatCalculations, (LPVOID)&count, 0, &threadResults[i + logicalProcessorCount / 2]);
    }

    // Wait for all threads to complete
    WaitForMultipleObjects(logicalProcessorCount, threads, TRUE, INFINITE);

    // Collect results
    for (int i = 0; i < logicalProcessorCount / 2; i++) {
        resultInt += threadResults[i];
        resultDouble += (double)threadResults[i + logicalProcessorCount / 2];  // Cast as needed for floating point
    }

    clock_t end = clock();
    long elapsed = (end - start) * 1000 / CLOCKS_PER_SEC;

    free(threads);
    free(threadResults);

    double score = ((double)count * logicalProcessorCount) / elapsed / 10000;
    printf("MultiThread Score: %.2f\n", score);
    return score;
}
#else
void* MultiThreadedIntegerCalculations(void* param) {
    long long count = *((long long*)param);
    long long result = PerformIntegerCalculations(count);
    return (void*)result;
}

void* MultiThreadedFloatCalculations(void* param) {
    long long count = *((long long*)param);
    double result = PerformFloatCalculations(count);
    return (void*)result;
}

long long MultiThreadedTest(long long count, int logicalProcessorCount) {
    clock_t start = clock();

    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * logicalProcessorCount);
    void** threadResults = (void**)malloc(sizeof(void*) * logicalProcessorCount);

    long long resultInt = 0;
    double resultDouble = 0.0;

    // Launch threads for integer and floating point calculations
    for (int i = 0; i < logicalProcessorCount / 2; i++) {
        pthread_create(&threads[i], NULL, MultiThreadedIntegerCalculations, (void*)&count);
        pthread_create(&threads[i + logicalProcessorCount / 2], NULL, MultiThreadedFloatCalculations, (void*)&count);
    }

    // Wait for all threads to complete
    for (int i = 0; i < logicalProcessorCount; i++) {
        void* result;
        pthread_join(threads[i], &result);
        if (i < logicalProcessorCount / 2) {
            resultInt += (long long)result;
        }
        else {
            resultDouble += (double)result;
        }
    }

    clock_t end = clock();
    long elapsed = (end - start) * 1000 / CLOCKS_PER_SEC;

    free(threads);
    free(threadResults);

    double score = ((double)count * logicalProcessorCount) / elapsed / 10000;
    printf("MultiThread Score: %.2f\n", score);
    return score;
}
#endif

int getLogicalProcessorCount() {
#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
#elif defined(__linux__)
    return sysconf(_SC_NPROCESSORS_ONLN); // Linux
#elif defined(__APPLE__)
    int count;
    size_t size = sizeof(count);
    sysctlbyname("hw.logicalcpu", &count, &size, NULL, 0); // macOS
    return count;
#else
    return 1;  // Default case, in case of an unknown platform
#endif
}

int main() {
    long long countPerLogicalProcessor = 1000000000;

    int logicalProcessorCount = getLogicalProcessorCount();

    printf("Logical Processors: %d\n", logicalProcessorCount);
    printf("%lld calculations per logical processor\n", countPerLogicalProcessor);

    long long singleScore = SingleThreadedTest(countPerLogicalProcessor, 2); // Using 2 here to avoid JIT optimization

    if (logicalProcessorCount > 0) {
        long long multiScore = MultiThreadedTest(countPerLogicalProcessor, logicalProcessorCount * 2);
        printf("Ratio: %.2fx\n", (double)multiScore / singleScore);
    }
    else {
        printf("Omitted MultiThreaded Test because there is no point.\n");
    }

    return 0;
}
