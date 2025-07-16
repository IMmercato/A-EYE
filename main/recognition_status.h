#ifndef RECOGNITION_STATUS_H
#define RECOGNITION_STATUS_H

#include <stdbool.h> // For bool type
#include <string.h>  // For strncpy (if you use it directly in header for inline functions)

// Structure to hold the current face recognition status
typedef struct {
    bool recognized;      // True if a known face is recognized, false otherwise
    int  face_id;         // ID of the recognized face (-1 for unknown, 0, 1, 2... for known users)
    char name[32];        // Name of the recognized person (e.g., "John Doe"). Max 31 chars + null.
    // Add any other relevant status information here, e.g., confidence score, timestamp
} recognition_status_t;

// Declare the global status variable (defined in .c file)
extern recognition_status_t g_current_recognition_status;

// Function to update the global status (called by face recognition logic)
// NOTE: In a multi-threaded FreeRTOS environment,
//       this function should ideally use a mutex/semaphore to protect
//       g_current_recognition_status from race conditions.
void recognition_status_update(bool recognized, int id, const char* name);

#endif // RECOGNITION_STATUS_H