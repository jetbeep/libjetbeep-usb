plugins {
    application
}

application {
    mainClassName = "example.Main"
    applicationDefaultJvmArgs = listOf("-Djava.library.path=../libjetbeep-jni")    
}

dependencies {
    compile(project(":libjetbeep-jni-java"))
}

val jar by tasks.getting(Jar::class) {
    manifest {
        attributes["Main-Class"] = "example.Main"
    }
}

val run by tasks.getting(JavaExec::class) {
    standardInput = System.`in`
}