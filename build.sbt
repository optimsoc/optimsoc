scalaVersion := "2.11.6"

libraryDependencies += "edu.berkeley.cs" %% "chisel" % "latest.release"

unmanagedSourceDirectories in Compile <++= baseDirectory { base =>
  Seq(
    base / "interfaces/rocket",
    base / "interconnect/rocket",
    base / "blocks/buffer/rocket",
    base / "blocks/arbiter/rocket",
    base / "modules/rocket",
    base / "modules/mam/rocket",
    base / "modules/stm/rocket",
    base / "modules/ctm/rocket"
  )
}
