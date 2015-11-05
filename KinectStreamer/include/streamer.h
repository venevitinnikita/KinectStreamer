#ifndef STREAMER_H_
#define STREAMER_H_

#include <pthread.h>
#include <libfreenect.h>
#include <libusb.h>

// TODO streamer api functions for Java (init_func, get_depth (callback probably))

#define DEPTH_IMAGE_WIDTH 640
#define DEPTH_IMAGE_HEIGHT 480
#define DEPTH_BUFFER_SIZE DEPTH_IMAGE_WIDTH*DEPTH_IMAGE_HEIGHT*16

class Mutex
{
private:
	pthread_mutex_t m_mutex;
public:
	Mutex()
	{
		pthread_mutex_init(&m_mutex, NULL);
	}
	void lock()
	{
		pthread_mutex_lock(&m_mutex);
	}
	void unlock()
	{
		pthread_mutex_unlock(&m_mutex);
	}
	~Mutex()
	{
		pthread_mutex_destroy(&m_mutex);
	}
};

class Device
{
private:

	Mutex m_mutex;
	freenect_context *m_context;
	pthread_t m_thread;

	void (*m_callback)(uint16_t *);

	volatile bool m_stop;

	static void *pthread_callback(void *_device_loop)
	{
		Device *device = static_cast<Device *>(_device_loop);
		device->device_loop();
		pthread_exit(NULL);
	}

	freenect_device *m_device;

	static void freenect_depth_callback(freenect_device *dev, void *_depth,
			uint32_t timestamp)
	{
		uint16_t *depth_buffer = static_cast<uint16_t *>(_depth);
		Device *device = static_cast<Device *>(freenect_get_user(dev));
		device->kinect_callback(depth_buffer);
	}

public:

	Device(void (*_depth_callback)(uint16_t *)) :
			m_callback(_depth_callback), m_stop(false)
	{
		freenect_init(&m_context, NULL);
		freenect_select_subdevices(m_context,
				static_cast<freenect_device_flags>(FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));

		pthread_create(&m_thread, NULL, pthread_callback, (void*) this);

		freenect_open_device(m_context, &m_device, 0);
		freenect_set_user(m_device, this);
		freenect_set_depth_mode(m_device,
				freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM,
						FREENECT_DEPTH_11BIT));
		freenect_set_depth_callback(m_device, freenect_depth_callback);
	}

	void start()
	{
		freenect_set_led(m_device, LED_YELLOW);
		freenect_start_depth(m_device);
	}

	void stop()
	{
		freenect_stop_depth(m_device);
		freenect_set_led(m_device, LED_BLINK_GREEN);
	}

	~Device()
	{
		m_stop = true;
		pthread_join(m_thread, NULL);
		freenect_shutdown(m_context);
	}

	void kinect_callback(uint16_t *_depth_buffer)
	{
		(*m_callback)(_depth_buffer);
	}

	void device_loop()
	{
		while (!m_stop)
		{
			int res = freenect_process_events(m_context);

			if (res < 0)
			{
				if (res == LIBUSB_ERROR_INTERRUPTED)
				{
					continue;
				}
				m_stop = true;
			}
		}
	}
};

#endif /* STREAMER_H_ */
