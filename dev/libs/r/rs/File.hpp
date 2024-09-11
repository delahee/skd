#pragma once

namespace rd {
	struct Buffer;
}

//read and write files using std::filesystem::path
namespace rs {
	struct File {

		static std::string sep();
		static bool createDir(const std::string& path);
		static bool exists(const std::string& path);

		static bool listDir(const std::string& path, std::vector<std::string> & outPathes );
		static bool enumerate(const std::string& path, const std::string& ext, std::vector<std::string> & outFiles );

		//regular non res files read/write, use for assets/editor etc
		static bool read(const std::string& path, std::string& content);
		static bool read( const char * path, Str& content);

		static bool write(const char * path, const std::string & content);
		static bool write(const std::string & path, const std::string & content);
		static bool write(const char* path, const rd::Buffer & content);

		static bool readSaveFile(const std::string& path, std::string& content);
		static bool writeSaveFile(const std::string& path, const std::string& content);
		static bool copy(const std::string& from, const std::string& to);
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