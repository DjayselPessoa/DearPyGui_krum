#pragma once

#include <mutex>
#include "mvThreadPool.h"
#include "mvContext.h"

namespace Marvel {

	static PyObject* SanitizeCallback(PyObject* callback)
	{
		if (callback == Py_None)
			return nullptr;

		return callback;
	}

	class mvCallbackRegistry
	{

		static constexpr const int s_MaxNumberOfCalls = 50;

	public:

		static void InsertParser(std::map<std::string, mvPythonParser>* parsers);

		MV_CREATE_COMMAND(set_frame_callback);
		MV_CREATE_COMMAND(set_exit_callback);
		MV_CREATE_COMMAND(set_viewport_resize_callback);

		MV_START_COMMANDS
			MV_ADD_COMMAND(set_frame_callback);
			MV_ADD_COMMAND(set_exit_callback);
			MV_ADD_COMMAND(set_viewport_resize_callback);
		MV_END_COMMANDS

	public:

		void onFrame     (int frame);
		void runTasks    ();
        void runCallback (PyObject* callback, mvUUID sender, PyObject* app_data, PyObject* user_data);
        void runCallback (PyObject* callback, const std::string& sender, PyObject* app_data, PyObject* user_data);
        void addCallback (PyObject* callback, mvUUID sender, PyObject* app_data, PyObject* user_data);
        void addCallback (PyObject* callback, const std::string& sender, PyObject* app_data, PyObject* user_data);
		bool runCallbacks();
		void stop        () { m_running = false; }

		template<typename F, typename ...Args>
		std::future<typename std::invoke_result<F, Args...>::type> submit(F f)
		{

			typedef typename std::invoke_result<F, Args...>::type result_type;
			std::packaged_task<result_type()> task(std::move(f));
			std::future<result_type> res(task.get_future());

			if (GContext->started)
				m_tasks.push(std::move(task));
			else
				task();

			return res;
		}

		template<typename F, typename ...Args>
		std::future<typename std::invoke_result<F, Args...>::type> submitCallback(F f)
		{

			if (m_callCount > s_MaxNumberOfCalls)
			{
				return {};
			}

			m_callCount++;

			typedef typename std::invoke_result<F, Args...>::type result_type;
			std::packaged_task<result_type()> task(std::move(f));
			std::future<result_type> res(task.get_future());

			m_calls.push(std::move(task));

			return res;
		}

		//-----------------------------------------------------------------------------
        // Callbacks
        //----------------------------------------------------------------------------- 
        void setResizeCallback (PyObject* callback) { m_resizeCallback = SanitizeCallback(callback); }
        void setOnCloseCallback(PyObject* callback) { m_onCloseCallback = SanitizeCallback(callback); }

        [[nodiscard]] PyObject* getResizeCallback (){ return m_resizeCallback; }
        [[nodiscard]] PyObject* getOnCloseCallback(){ return m_onCloseCallback; }
	
	public:

		mvQueue<mvFunctionWrapper> m_tasks;
		mvQueue<mvFunctionWrapper> m_calls;
		std::atomic<bool>          m_running;
		std::atomic<int>           m_callCount = 0;

		// callbacks
		PyObject* m_resizeCallback = nullptr;
		PyObject* m_onCloseCallback = nullptr;

        int _highestFrame = 0;
        std::unordered_map<int,  PyObject*> _frameCallbacks;
        

	};

}
