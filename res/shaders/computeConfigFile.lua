shaderConfig =
{
	rootPath = "/",
	shaderUses =
	{
		{
			shaderPath = "glsl/UpdateParticle_cs.glsl",
			shaderStage = 2,
			defines = { "PASTA_LOCAL_SIZE_X=1024" },
			variations = { "PARTICULE" },
		},
		{
			shaderPath = "glsl/Particle_vs.glsl",
			shaderStage = 0,
			defines = {},
			variations = { "PARTICULE" },
		},
		{
			shaderPath = "glsl/Particle_fs.glsl",
			shaderStage = 1,
			defines = {},
			variations = {},
		}
	},
	variations =
	{
		{
			name = "PARTICULE",
			defines =
			{
				{ "MAX_PARTICLES=1024" },
				{ "MAX_PARTICLES=1024", "HAS_TEXTURE" },
				{ "MAX_PARTICLES=1024", "HAS_COLOR_DATA" },
				{ "MAX_PARTICLES=1024", "HAS_COLOR_DATA", "HAS_TEXTURE" },
				{ "MAX_PARTICLES=2048" },
				{ "MAX_PARTICLES=2048", "HAS_TEXTURE" },
				{ "MAX_PARTICLES=2048", "HAS_COLOR_DATA" },
				{ "MAX_PARTICLES=2048", "HAS_COLOR_DATA", "HAS_TEXTURE" },
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
		batPath = "convertComputeParticle.bat",
		includeDirPath = "..",
		outputDirPath = "switch",
		vsVersion = "430",
		fsVersion = "430",
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