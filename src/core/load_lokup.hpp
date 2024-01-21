#pragma once
#include <Windows.h>
#include <threads.hpp>
#include <vector>

namespace tmgr {


	inline std::vector<std::pair<size_t, std::thread::id>> ThreadTime(thread_objects_t& threads) {
		std::vector<std::pair<size_t, std::thread::id>> result;
		auto chontextes = threads.get_contextes();

		union {
			struct { FILETIME CreationTime, ExitTime, KernelTime, UserTime; } file_time;
			struct { uint64_t CreationTime, ExitTime, KernelTime, UserTime; } time;
		} values;
		HANDLE thread_handle;
		for (auto it : chontextes) {
			thread_handle = OpenThread(THREAD_QUERY_INFORMATION,false,std::stoul((std::stringstream() << it.id).str()));
			if (thread_handle) {
				if (GetThreadTimes(
						thread_handle,
						&values.file_time.CreationTime,
						&values.file_time.ExitTime,
						&values.file_time.KernelTime,
						&values.file_time.UserTime
				))
					result.push_back({ values.time.KernelTime + values.time.UserTime,it.id });
				else
					result.push_back({ 0,it.id });
				CloseHandle(thread_handle);
			}
			else 
				result.push_back({ 0,it.id });
		}
		return result;
	}


	inline std::vector<std::pair<size_t, std::thread::id>> ThreadLoad(thread_objects_t& threads, std::vector<std::pair<size_t, std::thread::id>>& threads_time) {
		std::vector<std::pair<size_t, std::thread::id>> result;
		std::vector<std::pair<size_t, std::thread::id>> to_push;
		for (std::pair<size_t, std::thread::id> i : ThreadTime(threads))
		{
			auto found_item = std::find_if(threads_time.begin(), threads_time.end(), [i](const std::pair<size_t, std::thread::id>& comparer) {return comparer.second == i.second;});
			if (found_item == threads_time.end()){
				result.push_back(i);
				to_push.push_back(i);
				continue; 
			}
			result.push_back({ i.first - found_item->first,i.second });
			to_push.push_back(i);
		}
		threads_time = to_push;
		return result;
	}



	inline std::vector<std::pair<double, std::thread::id>> ThreadLoadToPercent(const std::vector<std::pair<size_t, std::thread::id>>& threads_load) {
		std::vector<std::pair<double, std::thread::id>> result;
		double all = 0;
		for (auto i : threads_load)
			all += i.first;
		for (auto i : threads_load)
			result.push_back({ 100 / all * i.first,i.second });
		return result;
	}

	inline std::vector<std::pair<double, std::thread::id>> ThreadLoadPercent(thread_objects_t& threads, std::vector<std::pair<size_t, std::thread::id>>& threads_time) {
		return tmgr::ThreadLoadToPercent(tmgr::ThreadLoad(threads, threads_time));
	}


	inline std::vector<std::pair<size_t, std::pair<std::thread::id, std::string>>> ThreadNamedTime(thread_objects_t& threads) {
		std::vector<std::pair<size_t,std::pair<std::thread::id, std::string>>> result;
		auto chontextes = threads.get_contextes();

		union {
			struct { FILETIME CreationTime, ExitTime, KernelTime, UserTime; } file_time;
			struct { uint64_t CreationTime, ExitTime, KernelTime, UserTime; } time;
		} values;
		HANDLE thread_handle;
		for (auto it : chontextes) {
			thread_handle = OpenThread(THREAD_QUERY_INFORMATION, false, std::stoul((std::stringstream() << it.id).str()));
			if (thread_handle) {
				if (GetThreadTimes(
					thread_handle,
					&values.file_time.CreationTime,
					&values.file_time.ExitTime,
					&values.file_time.KernelTime,
					&values.file_time.UserTime
				))
					result.push_back({ values.time.KernelTime + values.time.UserTime,{it.id,it.name } });
				else
					result.push_back({ 0,{it.id,it.name } });
				CloseHandle(thread_handle);
			}
			else
				result.push_back({ 0,{it.id,it.name } });
		}
		return result;
	}


	inline std::vector<std::pair<size_t, std::pair<std::thread::id,std::string>>> ThreadNamedLoad(thread_objects_t& threads, std::vector<std::pair<size_t, std::pair<std::thread::id, std::string>>>& threads_time) {
		std::vector<std::pair<size_t, std::pair<std::thread::id,std::string>>> result;
		std::vector<std::pair<size_t, std::pair<std::thread::id,std::string>>> to_push;
		for (std::pair<size_t, std::pair<std::thread::id,std::string>> i : ThreadNamedTime(threads))
		{
			auto found_item = std::find_if(threads_time.begin(), threads_time.end(), [i](const std::pair<size_t, std::pair<std::thread::id, std::string>>& comparer) {return comparer.second.first == i.second.first; });
			if (found_item == threads_time.end()) {
				result.push_back(i);
				to_push.push_back(i);
				continue;
			}
			result.push_back({ i.first - found_item->first,i.second });
			to_push.push_back(i);
		}
		threads_time = to_push;
		return result;
	}



	inline std::vector<std::pair<double, std::pair<std::thread::id, std::string>>> ThreadNamedLoadToPercent(const std::vector< std::pair<size_t, std::pair<std::thread::id, std::string>> >& threads_load) {
		std::vector<std::pair<double, std::pair<std::thread::id, std::string>>> result;
		double all = 0;
		for (auto i : threads_load)
			all += i.first;
		for (auto i : threads_load)
			result.push_back({ 100 / all * i.first,i.second });
		return result;
	}

	inline std::vector<std::pair<double, std::pair<std::thread::id, std::string>>> ThreadNamedLoadPercent(thread_objects_t& threads, std::vector<std::pair<size_t, std::pair<std::thread::id, std::string>>>& threads_time) {
		return tmgr::ThreadNamedLoadToPercent( tmgr::ThreadNamedLoad(threads, threads_time));
	}

}



