package com.selex.bssStation.service

class ThreadProcessBss: Runnable {
    override fun run() {
        for (i in generateSequence(0) { it }) {
            processBssFromJNY()
        }

    }
}