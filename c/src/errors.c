#include "../include/errors.h"

void init_error(error_t *error)
{
    error->count = 0;
    error->max_severity = NON_CRITICAL_ERROR;
    for (int i = 0; i < MAX_ERRORS; i++)
    {
        error->errors[i].code = NULL;
        error->errors[i].severity = 0;
        error->errors[i].message = NULL;
        error->errors[i].location = NULL;
    }
    error->aggregated_message[0] = '\0';
}

void add_error(error_t *error, const char *err_code, error_severity_t severity, const char *message, const char *location)
{
    if (error->count < MAX_ERRORS)
    {
        error->errors[error->count].code = err_code;
        error->errors[error->count].severity = severity;
        error->errors[error->count].message = message;
        error->errors[error->count].location = location;
        error->count++;

        if (severity == CRITICAL_ERROR)
        {
            error->max_severity = CRITICAL_ERROR;
        }

        snprintf(error->aggregated_message + strlen(error->aggregated_message),
                 sizeof(error->aggregated_message) - strlen(error->aggregated_message),
                 "Error in %s: %s (Code: %s)\n", location, message, err_code);
    }
}

void report_errors(error_t *error, void (*callback)(const char *, int))
{
    if (callback != NULL)
    {
        callback(error->aggregated_message, error->max_severity);
    }
}