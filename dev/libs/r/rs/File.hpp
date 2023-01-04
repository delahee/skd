#pragma once

#include <string>

//read and write files using std::filesystem::path
namespace rs {
	struct File {

		static std::string sep();
		static bool createDir(const std::string& path);
		static bool exists(const std::string& path, std::string& content);
		//regular non res files read/write
		static bool read(const std::string& path, std::string& content);
		static bool write(const std::string & path, const std::string & content);

		static bool readSaveFile(const std::string& path, std::string& content);
		static bool writeSaveFile(const std::string& path, const std::string& content);
	};

	class FileHost : public rd::Agent {
	public:
		double			timer = 0;
		Pasta::u64		writeTime = 0;
		std::string		path;
		std::string		cpath;
		std::function<void(const char*)> 
						onEvalTime = 0;
		int				nbSucessfullProcess = 0;

#if defined(PASTA_FINAL) || !defined(PASTA_WIN)
		static inline constexpr bool ENABLED = false;
#else 
		static inline constexpr bool ENABLED = true;
#endif

								FileHost(const char* _path, std::function<void(const char*)> _onEvalTime, bool doProcessNow = false);

		void					process();
		void					update(double dt)override;

		static rs::FileHost*	add( const char* _path, std::function<void(const char*)> _onEvalTime );
	};
}