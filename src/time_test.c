#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    // Get initial time
    time_t current_time;
    time(&current_time);
    struct tm* local_time = localtime(&current_time);
    
    char time_str[26];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
    printf("Current time: %s\n", time_str);

    // Wait 5 seconds
    sleep(5);
    
    // Get new time
    time_t later_time;
    time(&later_time);
    struct tm* later_local_time = localtime(&later_time);
    
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", later_local_time);
    printf("Later time: %s\n", time_str);
    
    // Calculate difference
    double diff = difftime(later_time, current_time);
    printf("Time difference: %.2f seconds\n", diff);
    
    return 0;
}