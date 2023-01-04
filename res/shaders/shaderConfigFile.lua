shaderConfig =
{
	rootPath = "/",
	shaderUses =
	{
		{
			shaderPath = "glsl/Basic_vs.glsl",
			shaderStage = 0,
			defines = {},
			variations =
			{
				"BASIC"
			},
		},
		{
			shaderPath = "glsl/Basic_fs.glsl",
			shaderStage = 1,
			defines = {},
			variations =
			{
				"BASIC"
			},
		},
		{
			shaderPath = "glsl/MSDF_vs.glsl",
			shaderStage = 0,
			defines = {},
			variations =
			{
				"BASIC"
			},
		},
		{
			shaderPath = "glsl/MSDF_fs.glsl",
			shaderStage = 1,
			defines = {},
			variations =
			{
				"BASIC"
			},
		},
		{
			shaderPath = "glsl/Uber_vs.glsl",
			shaderStage = 0,
			defines = {},
			variations =
			{
				"BASIC"
			},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = {},
			variations =
			{
				"BASIC"
			},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_VIGNETTE" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_BLOOM_PYRAMID" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_BLOOM_PYRAMID", "HAS_VIGNETTE" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_DISTORTION", "HAS_BLOOM_PYRAMID", "HAS_VIGNETTE" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_BLOOM_PYRAMID", "HAS_GLITCH" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_DISTORTION", "HAS_GLITCH" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_BLOOM_PYRAMID", "HAS_DISTORTION" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_BLOOM_PYRAMID", "HAS_DISTORTION", "HAS_GLITCH" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_BLOOM_PYRAMID", "HAS_CHROMATIC_ABERRATION" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_BLOOM_PYRAMID", "HAS_CHROMATIC_ABERRATION", "HAS_GLITCH" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_BLOOM_PYRAMID", "HAS_VIGNETTE", "HAS_CHROMATIC_ABERRATION" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_BLOOM_PYRAMID", "HAS_VIGNETTE", "HAS_CHROMATIC_ABERRATION", "HAS_GLITCH" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_BLOOM_PYRAMID", "HAS_DISTORTION", "HAS_VIGNETTE", "HAS_CHROMATIC_ABERRATION" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_BLOOM_PYRAMID", "HAS_DISTORTION", "HAS_VIGNETTE", "HAS_CHROMATIC_ABERRATION", "HAS_GLITCH" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_BLOOM_PYRAMID", "HAS_DISTORTION", "HAS_COLOR_MATRIX", "HAS_VIGNETTE", "HAS_CHROMATIC_ABERRATION", "HAS_GLITCH" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "UNIFORM_COLOR", "TEXTURE", "HAS_GLITCH" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "UNIFORM_COLOR", "TEXTURE", "KILL_ALPHA", "HAS_COLOR_ADD" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "UNIFORM_COLOR", "TEXTURE", "HAS_GLITCH", "HAS_COLOR_ADD" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "UNIFORM_COLOR", "TEXTURE", "KILL_ALPHA", "HAS_GLITCH", "HAS_COLOR_ADD" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "KILL_ALPHA", "HAS_GLITCH", "HAS_COLOR_ADD" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "UNIFORM_COLOR", "TEXTURE", "KILL_ALPHA", "HAS_GLITCH", "HAS_COLOR_MATRIX" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "UNIFORM_COLOR", "TEXTURE", "KILL_ALPHA", "HAS_GLITCH", "HAS_COLOR_MATRIX", "HAS_COLOR_ADD" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "UNIFORM_COLOR", "TEXTURE", "HAS_GLITCH", "HAS_COLOR_MATRIX", "HAS_COLOR_ADD" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "KILL_ALPHA", "HAS_GLITCH", "HAS_COLOR_MATRIX", "HAS_COLOR_ADD" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_GBLUR" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_COLOR_MATRIX" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_COLOR_MATRIX", "HAS_VIGNETTE" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_COLOR_MATRIX", "HAS_VIGNETTE", "HAS_CHROMATIC_ABERRATION" },
			variations = {},
		},
		{
			shaderPath = "glsl/Uber_fs.glsl",
			shaderStage = 1,
			defines = { "TEXTURE", "PREMUL_ALPHA", "HAS_COLOR_MATRIX", "HAS_VIGNETTE", "HAS_CHROMATIC_ABERRATION", "HAS_GLITCH" },
			variations = {},
		},
		{
			shaderPath = "glsl/BloomPyramid_vs.glsl",
			shaderStage = 0,
			defines = {},
			variations = {},
		},
		{
			shaderPath = "glsl/BloomPyramid_fs.glsl",
			shaderStage = 1,
			defines = {},
			variations =
			{
				"BLOOM"
			},
		},
		{
			shaderPath = "glsl/Arrow_vs.glsl",
			shaderStage = 0,
			defines = {},
			variations = {},
		},
		{
			shaderPath = "glsl/Arrow_fs.glsl",
			shaderStage = 1,
			defines = {},
			variations = {},
		},
		{
			shaderPath = "glsl/Fog_vs.glsl",
			shaderStage = 0,
			defines = {},
			variations = {},
		},
		{
			shaderPath = "glsl/Fog_fs.glsl",
			shaderStage = 1,
			defines = {},
			variations = {},
		},
		{
			shaderPath = "glsl/Fresnel_vs.glsl",
			shaderStage = 0,
			defines = {},
			variations = {},
		},
		{
			shaderPath = "glsl/Fresnel_fs.glsl",
			shaderStage = 1,
			defines = {},
			variations = {},
		},
		{
			shaderPath = "glsl/GodRay_vs.glsl",
			shaderStage = 0,
			defines = {},
			variations = {},
		},
		{
			shaderPath = "glsl/GodRay_fs.glsl",
			shaderStage = 1,
			defines = {},
			variations = {},
		},
		{
			shaderPath = "glsl/Path_vs.glsl",
			shaderStage = 0,
			defines = {},
			variations = {"VIEW_THROUGHABLE"},
		},
		{
			shaderPath = "glsl/Path_fs.glsl",
			shaderStage = 1,
			defines = {},
			variations = {"VIEW_THROUGHABLE"},
		},
		{
			shaderPath = "glsl/AttackRange_vs.glsl",
			shaderStage = 0,
			defines = {},
			variations = {"VIEW_THROUGHABLE"},
		},
		{
			shaderPath = "glsl/AttackRange_fs.glsl",
			shaderStage = 1,
			defines = {},
			variations = {"VIEW_THROUGHABLE"},
		},
		{
			shaderPath = "glsl/Room_vs.glsl",
			shaderStage = 0,
			defines = {},
			variations = {},
		},
		{
			shaderPath = "glsl/Room_fs.glsl",
			shaderStage = 1,
			defines = {},
			variations = {},
		},
		{
			shaderPath = "glsl/Room_vs.glsl",
			shaderStage = 0,
			defines = {"EARLY_DEPTH"},
			variations = {},
		},
		{
			shaderPath = "glsl/Room_fs.glsl",
			shaderStage = 1,
			defines = {"EARLY_DEPTH"},
			variations = {},
		},
		{
			shaderPath = "glsl/Shield_vs.glsl",
			shaderStage = 0,
			defines = {},
			variations = {},
		},
		{
			shaderPath = "glsl/Shield_fs.glsl",
			shaderStage = 1,
			defines = {},
			variations = {},
		},
		{
			shaderPath = "glsl/ViewThrough_vs.glsl",
			shaderStage = 0,
			defines = {},
			variations = {},
		},
		{
			shaderPath = "glsl/ViewThrough_fs.glsl",
			shaderStage = 1,
			defines = {},
			variations = {},
		}
	},
	variations =
	{
		{
			name = "BASIC",
			defines =
			{
				{},
				
				{ "VERTEX_COLOR" },
				{ "UNIFORM_COLOR" },
				{ "TEXTURE" },
				{ "PREMUL_ALPHA" },
				{ "VERTEX_COLOR", "UNIFORM_COLOR" },
				{ "VERTEX_COLOR", "TEXTURE" },
				{ "VERTEX_COLOR", "PREMUL_ALPHA" },
				{ "UNIFORM_COLOR", "TEXTURE" },
				{ "UNIFORM_COLOR", "PREMUL_ALPHA" },
				{ "TEXTURE", "PREMUL_ALPHA" },
				{ "VERTEX_COLOR", "UNIFORM_COLOR", "TEXTURE" },
				{ "VERTEX_COLOR", "UNIFORM_COLOR", "PREMUL_ALPHA" },
				{ "VERTEX_COLOR", "TEXTURE", "PREMUL_ALPHA" },
				{ "UNIFORM_COLOR", "TEXTURE", "PREMUL_ALPHA" },
				{ "VERTEX_COLOR", "UNIFORM_COLOR", "TEXTURE", "PREMUL_ALPHA" },
				
				{ "KILL_ALPHA" },
				{ "VERTEX_COLOR", "KILL_ALPHA" },
				{ "UNIFORM_COLOR", "KILL_ALPHA" },
				{ "TEXTURE", "KILL_ALPHA" },
				{ "PREMUL_ALPHA", "KILL_ALPHA" },
				{ "VERTEX_COLOR", "UNIFORM_COLOR", "KILL_ALPHA" },
				{ "VERTEX_COLOR", "TEXTURE", "KILL_ALPHA" },
				{ "VERTEX_COLOR", "PREMUL_ALPHA", "KILL_ALPHA" },
				{ "UNIFORM_COLOR", "TEXTURE", "KILL_ALPHA" },
				{ "UNIFORM_COLOR", "PREMUL_ALPHA", "KILL_ALPHA" },
				{ "TEXTURE", "PREMUL_ALPHA", "KILL_ALPHA" },
				{ "VERTEX_COLOR", "UNIFORM_COLOR", "TEXTURE", "KILL_ALPHA" },
				{ "VERTEX_COLOR", "UNIFORM_COLOR", "PREMUL_ALPHA", "KILL_ALPHA" },
				{ "VERTEX_COLOR", "TEXTURE", "PREMUL_ALPHA", "KILL_ALPHA" },
				{ "UNIFORM_COLOR", "TEXTURE", "PREMUL_ALPHA", "KILL_ALPHA" },
				{ "VERTEX_COLOR", "UNIFORM_COLOR", "TEXTURE", "PREMUL_ALPHA", "KILL_ALPHA" },
				
				{ "TEXTURE_ALPHA_ONLY" },
				{ "VERTEX_COLOR", "TEXTURE_ALPHA_ONLY" },
				{ "UNIFORM_COLOR", "TEXTURE_ALPHA_ONLY" },
				{ "TEXTURE", "TEXTURE_ALPHA_ONLY" },
				{ "PREMUL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "VERTEX_COLOR", "UNIFORM_COLOR", "TEXTURE_ALPHA_ONLY" },
				{ "VERTEX_COLOR", "TEXTURE", "TEXTURE_ALPHA_ONLY" },
				{ "VERTEX_COLOR", "PREMUL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "UNIFORM_COLOR", "TEXTURE", "TEXTURE_ALPHA_ONLY" },
				{ "UNIFORM_COLOR", "PREMUL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "TEXTURE", "PREMUL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "VERTEX_COLOR", "UNIFORM_COLOR", "TEXTURE", "TEXTURE_ALPHA_ONLY" },
				{ "VERTEX_COLOR", "UNIFORM_COLOR", "PREMUL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "VERTEX_COLOR", "TEXTURE", "PREMUL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "UNIFORM_COLOR", "TEXTURE", "PREMUL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "VERTEX_COLOR", "UNIFORM_COLOR", "TEXTURE", "PREMUL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				
				{ "KILL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "VERTEX_COLOR", "KILL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "UNIFORM_COLOR", "KILL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "TEXTURE", "KILL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "PREMUL_ALPHA", "KILL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "VERTEX_COLOR", "UNIFORM_COLOR", "KILL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "VERTEX_COLOR", "TEXTURE", "KILL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "VERTEX_COLOR", "PREMUL_ALPHA", "KILL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "UNIFORM_COLOR", "TEXTURE", "KILL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "UNIFORM_COLOR", "PREMUL_ALPHA", "KILL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "TEXTURE", "PREMUL_ALPHA", "KILL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "VERTEX_COLOR", "UNIFORM_COLOR", "TEXTURE", "KILL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "VERTEX_COLOR", "UNIFORM_COLOR", "PREMUL_ALPHA", "KILL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "VERTEX_COLOR", "TEXTURE", "PREMUL_ALPHA", "KILL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "UNIFORM_COLOR", "TEXTURE", "PREMUL_ALPHA", "KILL_ALPHA", "TEXTURE_ALPHA_ONLY" },
				{ "VERTEX_COLOR", "UNIFORM_COLOR", "TEXTURE", "PREMUL_ALPHA", "KILL_ALPHA", "TEXTURE_ALPHA_ONLY" },
			}
		},
		{
			name = "BLOOM",
			defines =
			{
				{ "PYRAMID_THRESHOLD" },
				{ "PYRAMID_DOWNSAMPLE" },
				{ "PYRAMID_UPSAMPLE" },
			}
		},
		{
			name = "VIEW_THROUGHABLE",
			defines =
			{
				{ },
				{ "VIEW_THROUGH" }
			}
		}
	},
	vulkanConfig =
	{
		batPath = "",
		glslcExePath = "",
		spirvExePath = "",
		findBindingsExePath = "",
		includeDirPath = "",
	},
	nxConfig =
	{
		batPath = "convert.bat",
		includeDirPath = "..",
		outputDirPath = "switch",
		vsVersion = "140",
		fsVersion = "330",
		csVersion = "430",
	},
	orbisConfig =
	{
		batPath = "",
		includeDirPath = "",
	},
	xboxOneConfig =
	{
		batPath = "",
		includeDirPath = "",
		glslcExePath = "",
		spv2hlslExePath = "",
		dxcExePath = "",
	},
}