@ECHO OFF
curl "https://docs.google.com/spreadsheets/d/1fEss2O1J_jnb7ge70ciZAR59_4sceJwKq0i6NY8jt54/export?gid=1226490109&format=tsv" > ../res/data/proto_chars.tsv
curl "https://docs.google.com/spreadsheets/d/1fEss2O1J_jnb7ge70ciZAR59_4sceJwKq0i6NY8jt54/export?gid=35085740&format=tsv" > ../res/data/proto_weapons.tsv
curl "https://docs.google.com/spreadsheets/d/1fEss2O1J_jnb7ge70ciZAR59_4sceJwKq0i6NY8jt54/export?gid=100821091&format=tsv" > ../res/data/proto_tuto.tsv