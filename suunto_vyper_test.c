#include <stdio.h>	// fopen, fwrite, fclose
#include <stdlib.h>	// atoi

#include "suunto.h"
#include "serial.h"
#include "utils.h"

#define WARNING(expr) \
{ \
	message ("%s:%d: %s\n", __FILE__, __LINE__, expr); \
}

int test_dump_sdm (const char* name, unsigned int delay)
{
	vyper *device = NULL;

	message ("suunto_vyper_open\n");
	int rc = suunto_vyper_open (&device, name);
	if (rc != SUUNTO_SUCCESS) {
		WARNING ("Error opening serial port.");
		return rc;
	}

	suunto_vyper_set_delay (device, delay);

	message ("suunto_vyper_detect_interface\n");
	rc = suunto_vyper_detect_interface (device);
	if (rc != SUUNTO_SUCCESS) {
		WARNING ("Interface not found.");
		suunto_vyper_close (device);
		return rc;
	}

	message ("suunto_vyper_read_dives\n");
	rc = suunto_vyper_read_dives (device, NULL, NULL);
	if (rc != SUUNTO_SUCCESS) {
		WARNING ("Cannot read dives.");
		suunto_vyper_close (device);
		return rc;
	}

	message ("suunto_vyper_close\n");
	rc = suunto_vyper_close (device);
	if (rc != SUUNTO_SUCCESS) {
		WARNING ("Cannot close device.");
		return rc;
	}

	return SUUNTO_SUCCESS;
}

int test_dump_memory (const char* name, unsigned int delay, const char* filename)
{
	unsigned char data[SUUNTO_VYPER_MEMORY_SIZE] = {0};
	vyper *device = NULL;

	message ("suunto_vyper_open\n");
	int rc = suunto_vyper_open (&device, name);
	if (rc != SUUNTO_SUCCESS) {
		WARNING ("Error opening serial port.");
		return rc;
	}

	suunto_vyper_set_delay (device, delay);

	message ("suunto_vyper_detect_interface\n");
	rc = suunto_vyper_detect_interface (device);
	if (rc != SUUNTO_SUCCESS) {
		WARNING ("Interface not found.");
		suunto_vyper_close (device);
		return rc;
	}

	message ("suunto_vyper_read_memory\n");
	rc = suunto_vyper_read_memory (device, 0x00, data, sizeof (data));
	if (rc != SUUNTO_SUCCESS) {
		WARNING ("Cannot read memory.");
		suunto_vyper_close (device);
		return rc;
	}

	message ("Dumping data\n");
	FILE* fp = fopen (filename, "wb");
	if (fp != NULL) {
		fwrite (data, sizeof (unsigned char), sizeof (data), fp);
		fclose (fp);
	}

	message ("suunto_vyper_close\n");
	rc = suunto_vyper_close (device);
	if (rc != SUUNTO_SUCCESS) {
		WARNING ("Cannot close device.");
		return rc;
	}

	return SUUNTO_SUCCESS;
}

const char* errmsg (int rc)
{
	switch (rc) {
	case SUUNTO_SUCCESS:
		return "Success";
	case SUUNTO_ERROR:
		return "Generic error";
	case SUUNTO_ERROR_IO:
		return "Input/output error";
	case SUUNTO_ERROR_MEMORY:
		return "Memory error";
	case SUUNTO_ERROR_PROTOCOL:
		return "Protocol error";
	case SUUNTO_ERROR_TIMEOUT:
		return "Timeout";
	default:
		return "Unknown error";
	}
}

int main(int argc, char *argv[])
{
	message_set_logfile ("VYPER.LOG");

#ifdef _WIN32
	const char* name = "COM1";
#else
	const char* name = "/dev/ttyS0";
#endif
	
	unsigned int delay = 500;

	if (argc > 2) {
		name = argv[1];
		delay = atoi (argv[2]);
	} else if (argc > 1) {
		name = argv[1];
	}

	message ("DEVICE=%s, DELAY=%i\n", name, delay);

	int a = test_dump_sdm (name, delay);
	int b = test_dump_memory (name, delay, "VYPER.DMP");

	message ("\nSUMMARY\n");
	message ("-------\n");
	message ("test_dump_sdm:    %s\n", errmsg (a));
	message ("test_dump_memory: %s\n", errmsg (b));

	message_set_logfile (NULL);

	return 0;
}
