import org.gradle.internal.os.OperatingSystem

val osName = System.getProperty("os.name").toLowerCase().replace(" ", "_")
val osArch = System.getProperty("os.arch").toLowerCase().replace(" ", "_")

plugins {
    application
    id("org.kordamp.markdown.convert") version "1.2.0"
}

application {
    mainClassName = "example.Main"
    applicationDefaultJvmArgs = listOf("-Djava.library.path=../libjetbeep-jni")    
}

dependencies {
    implementation(project(":libjetbeep-jni-java"))
}

val jar by tasks.getting(Jar::class) {
    manifest {
        attributes["Main-Class"] = "example.Main"
    }
}

val run by tasks.getting(JavaExec::class) {
    standardInput = System.`in`
}

var mdpdfCmdName = "";
if (OperatingSystem.current().isWindows()) {
    mdpdfCmdName = "mdpdf.cmd"
} else {
    mdpdfCmdName = "mdpdf"
}
val mdpdf by tasks.register<Exec>("mdpdf") {
    commandLine = listOf(mdpdfCmdName, "README.md")
}

tasks.distZip {
    archiveName = "libjetbeep-jni-$version-$osName-$osArch.zip"
    dependsOn(mdpdf)
}

distributions {
    main {
        contents {
            from("../libjetbeep-jni") {
                include(listOf("*.dylib", "*.so", "*.dll"))
                into("libjetbeep-jni")                    
            }
            into(".") {
                from(listOf("README.md", "README.pdf"))
            }
            into("docs") {
                from(listOf("../libjetbeep-jni-java/build/docs"))
            }
            from("src") {
                into("src")
            }
        }
    }
}