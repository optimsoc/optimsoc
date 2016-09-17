scalaVersion := "2.11.6"

libraryDependencies += "edu.berkeley.cs" %% "chisel" % "latest.release"

unmanagedSourceDirectories in Compile <++= baseDirectory { base =>
  Seq(
    base / "interfaces/rocket",
    base / "interconnect/rocket",
    base / "blocks/buffer/rocket",
    base / "blocks/arbiter/rocket",
    base / "modules/ccm/riscv/rocket",
    base / "modules/mam/tilelink",
    base / "modules/stm/riscv/rocket",
    base / "modules/ctm/riscv/rocket"
  )
}
