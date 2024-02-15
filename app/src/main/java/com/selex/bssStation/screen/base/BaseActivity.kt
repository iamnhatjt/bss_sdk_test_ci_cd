package com.selex.bssStation.screen.base


import android.app.admin.DevicePolicyManager
import android.content.IntentFilter
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.WindowCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
import com.selex.bssStation.BuildConfig
import com.selex.bssStation.core.utils.*
import dagger.hilt.android.AndroidEntryPoint
import org.greenrobot.eventbus.EventBus
import org.greenrobot.eventbus.Subscribe
import org.greenrobot.eventbus.ThreadMode
import javax.inject.Inject


@AndroidEntryPoint
abstract class BaseActivity : AppCompatActivity() {

    protected var hasNetwork: Boolean by Preference(Constants.HAS_NETWORK_KEY, true)

    private var mNetworkChangeReceiver: NetworkChangeReceiver? = null

    @Inject
    lateinit var sharedPreferences: SharedPreferences

    private lateinit var dpm: DevicePolicyManager

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        dpm = getSystemService(DevicePolicyManager::class.java)
    }
    override fun onStart() {
        super.onStart()
        EventBus.getDefault().register(this)
    }

    override fun onResume() {
        super.onResume()

        val filter = IntentFilter()
        filter.addAction("android.net.conn.CONNECTIVITY_CHANGE")
        mNetworkChangeReceiver = NetworkChangeReceiver()
        registerReceiver(mNetworkChangeReceiver, filter)

        enterKioskMode()
        setImmersiveMode()
    }

    override fun onStop() {
        super.onStop()
        EventBus.getDefault().unregister(this)
    }

    protected fun enterKioskMode() {
        if(dpm.isLockTaskPermitted(packageName) && BuildConfig.FLAVOR == "pro") {
            startLockTask()
        }
    }

    protected fun exitKioskMode() {
        stopLockTask()
    }

    private fun setImmersiveMode() {
        WindowCompat.getInsetsController(window, window.decorView).apply {
            hide(WindowInsetsCompat.Type.systemBars())
            systemBarsBehavior = BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
        }
    }

    /**
     * Network Change
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    open fun onNetworkChangeEvent(event: NetworkChangeEvent) {
        hasNetwork = event.isConnected
    }

    override fun onPause() {
        if (mNetworkChangeReceiver != null) {
            unregisterReceiver(mNetworkChangeReceiver)
        }
        super.onPause()
    }

}


