#ifndef STREAMER_H_
#define STREAMER_H_

#include <pthread.h>
#include <libfreenect.h>
#include <libusb.h>

// TODO streamer api functions for Java (init_func, get_depth (callback probably))

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

	void (*m_callback)();

	volatile bool m_stop;

	static void *pthread_callback(void *_device_loop)
	{
		Device *device = static_cast<Device *>(_device_loop);
		device->device_loop();
		return NULL;
	}

	freenect_device *m_device;

	static void freenect_depth_callback(freenect_device *dev, void *depth,
			uint32_t timestamp)
	{
		// TODO convert data and call m_callback
		Device *device = static_cast<Device *>(freenect_get_user(dev));
		device->kinect_callback();
	}

public:

	Device(void (*_depth_callback)()) :
			m_callback(_depth_callback), m_stop(false)
	{
		freenect_init(&m_context, NULL);
		freenect_select_subdevices(m_context,
				static_cast<freenect_device_flags>(FREENECT_DEVICE_CAMERA));

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
		freenect_start_depth(m_device);
	}

	void stop()
	{
		freenect_stop_depth(m_device);
	}

	~Device()
	{
		m_stop = true;
		pthread_join(m_thread, NULL);
		freenect_shutdown(m_context);
	}

	void kinect_callback()
	{
		(*m_callback)();
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
