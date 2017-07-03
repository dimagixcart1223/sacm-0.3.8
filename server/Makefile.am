noinst_PROGRAMS = mpsvr
mpsvr_SOURCES = anticheat.cpp \
		console.cpp \
		filterscripts.cpp \
		format.cpp \
		gangzonepool.cpp \
		gamemodes.cpp \
		httpclient.cpp \
		main.cpp \
		menu.cpp \
		menupool.cpp \
		netgame.cpp \
		netrpc.cpp \
		object.cpp \
		objectpool.cpp \
		pickuppool.cpp \
		player.cpp \
		playerpool.cpp \
		plugins.cpp \
		query.cpp \
		rcon.cpp \
		rconrpc.cpp \
		runutil.cpp \
		scrcore.cpp \
		scrcustom.cpp \
		scrtimers.cpp \
		signer.cpp \
		specialbuild.cpp \
		textdrawpool.cpp \
		vehicle.cpp \
		vehiclepool.cpp
mpsvr_LDADD = amx/libamx.a ../raknet/libraknet.a
mpsvr_LDFLAGS = -ldl -lpthread
mpsvr_CPPFLAGS = -w -DLINUX -DSAMPSRV -DAMX_NODYNALOAD -Iamx
