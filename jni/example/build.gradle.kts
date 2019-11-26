plugins {
    application
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

distributions {
    main {
        contents {
            into("libjetbeep-jni") {
                from(listOf("../libjetbeep-jni/libjetbeep-jni.dylib", "../libjetbeep-jni/libjetbeep-jni.so", "../libjetbeep-jni/jetbeep-jni.dll"))
            }
            into("doc") {
                from(listOf("README.md", "README.pdf"))
            }
        }
    }
}