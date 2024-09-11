#pragma  once

namespace rd {
	class PathUtils {
	public:
		static std::string	removeAllExtensions(const std::string& p);
		static std::string	removeExtension(const std::string& p);
		static void			removeExtension(Str& p);

		static std::string basename(const std::string& p);


		/**
		* ex 
		* path : "res/editor/compo"
		* 
		**/
		static void listDir(const char* folder, const char* ext,std::vector<std::string> & res);
		static void listDir(const char* folder, const char* ext, eastl::vector<Str> & res);
	};
}