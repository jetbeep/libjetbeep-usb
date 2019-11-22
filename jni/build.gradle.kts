subprojects {
    version = "0.1.0-alpha"
}

val cmake = tasks.register<Exec>("cmake") {
    commandLine = listOf("cmake", "--build", ".", "--target", "jetbeep-jni", "--", "-j", "10")
    workingDir = file("../build")
}

tasks.register("build") {
    dependsOn(cmake)
}
