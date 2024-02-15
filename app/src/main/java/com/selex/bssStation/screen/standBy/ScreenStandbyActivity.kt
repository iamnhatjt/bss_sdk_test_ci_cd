package com.selex.bssStation.screen.standBy

import android.annotation.SuppressLint
import android.app.Activity
import android.content.Intent
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.MotionEvent
import android.view.View
import androidx.activity.viewModels
import androidx.databinding.DataBindingUtil
import androidx.recyclerview.widget.RecyclerView
import androidx.viewpager2.widget.ViewPager2
import com.chuckerteam.chucker.api.Chucker
import com.selex.bssStation.App
import com.selex.bssStation.R
import com.selex.bssStation.core.utility.showSnackbar
import com.selex.bssStation.core.utils.*
import com.selex.bssStation.databinding.ActivityScreenStandbyBinding
import com.selex.bssStation.model.banner.BannerModel
import com.selex.bssStation.model.bss.BssStatusModel
import com.selex.bssStation.screen.Screen
import com.selex.bssStation.screen.ScreenManager
import com.selex.bssStation.screen.base.BaseActivity
import com.selex.bssStation.screen.login.loginAccount.LoginAccountActivity
import com.selex.bssStation.screen.login.loginQr.LoginQRActivity
import com.selex.bssStation.service.BackgroundService
import com.selex.bssStation.service.NoActionCommon
import dagger.hilt.android.AndroidEntryPoint
import org.greenrobot.eventbus.Subscribe
import org.greenrobot.eventbus.ThreadMode
import setSimpleDateFormatHour
import java.util.*

@AndroidEntryPoint
class ScreenStandbyActivity : BaseActivity() {
    private val viewModel: ScreenStandbyViewModel by viewModels()
    private lateinit var _binding: ActivityScreenStandbyBinding

    private lateinit var  viewPager2: ViewPager2
    private lateinit var handler : Handler
    private lateinit var imageList:ArrayList<BannerModel>
    private lateinit var adapter: BannerAdapter
    private var runnableTimer: Runnable? = null

    private var timeDelay = 60000
    private var readyPack: Int = 0
    private var bssStatus: BssStatusModel? = null

    private var countTabTimeText = 0

    companion object {
        fun startActivity(activity: Activity, bundle: Bundle? = null) {
            val intent = Intent(activity, ScreenStandbyActivity::class.java)
            if (bundle != null) {
                intent.putExtras(bundle)
            }
            activity.startActivity(intent)
        }
    }
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        _binding = DataBindingUtil.setContentView(this, R.layout.activity_screen_standby)
        _binding.layoutMaintenance.visibility = View.GONE
        init()

        viewModel.response.observe(this, androidx.lifecycle.Observer {
            when (it) {
                is NetworkResult.Success -> {
                    if (it.data?.isNotEmpty() == true) {
                        it.data?.let { it1 -> adapter.updateListBanner(it1) };
                    }
                }
                is NetworkResult.Error -> {
                    if (adapter.imageList.first().bannerStatus == null) {
                        viewModel.getBanner()
                    }
                }
                else -> {}
            }
        })
        viewModel.getBssStatus()
        viewModel.getNumberPinAvailable()
        viewModel.getBanner()
        showNetworkDebug()

        viewModel.numberPin.observe(this) {
            handleNumberPin(it)
        }

        NoActionCommon.instance.listener = {
            sharedPreferences.setToken("")
            NoActionCommon.instance.cancelTimer()
            val intent = Intent(this, ScreenStandbyActivity::class.java)
            intent.flags = Intent.FLAG_ACTIVITY_CLEAR_TOP
            startActivity(intent)
        }

        viewModel.bssStatusResponse.observe(this) {
            if (it.data?.id != null) {
                BackgroundService.bss = it.data
                handleBatteryStationStatus(it.data)
            } else {
                viewModel.getBssStatus()
            }
        }

        viewModel.numberPinResponse.observe(this) {
            if (it.data != null) {
                it.data.desiredState?.cabinet?.ready_pack?.let { it1 -> handleNumberPin(it1) }
            } else {

            }
        }

        viewModel.showLayoutMaintain.observe(this) { isShow ->
            if(isShow) {
                showLayoutMaintain()
            }
        }

        viewPager2.registerOnPageChangeCallback(object : ViewPager2.OnPageChangeCallback(){
            override fun onPageSelected(position: Int) {
                super.onPageSelected(position)
                handler.removeCallbacks(runnable)
                handler.postDelayed(runnable , 5000)
            }
        })

        _binding.icStatus.setOnClickListener {
            countTabTimeText++
            if(countTabTimeText == 5) {
                countTabTimeText = 0
                LoginAccountActivity.startActivity(this@ScreenStandbyActivity)
            }
        }

        viewModel.showSnackbar.observe(this) {
            if (it == true) {
                showSnackbar(_binding.root, message = resources.getText(R.string.warning_dialog_login).toString())
            }
        }
    }

    override fun onPause() {
        super.onPause()

        handler.removeCallbacks(runnable)
    }

    override fun onResume() {
        super.onResume()
        sharedPreferences.setToken("")
        if (ScreenManager.currentScreen == Screen.RESULT_SUCCESS) {
            viewModel.getNumberPinAvailable()
        }
        ScreenManager.currentScreen = Screen.STAND_BY
        handler.postDelayed(runnable , 5000)
        BackgroundService.isNeedSyncData = true
        handleStatus()

        _binding.tvTime.text = setSimpleDateFormatHour(System.currentTimeMillis())
        handler.postDelayed(Runnable {
            handler.postDelayed(runnableTimer!!, timeDelay.toLong())
            //update TextView time 1 minute
            if (setSimpleDateFormatHour(System.currentTimeMillis()) == "23:59" && ScreenManager.currentScreen == Screen.STAND_BY) {
                Log.d(TAG, "chuanc mrn ${setSimpleDateFormatHour(System.currentTimeMillis())}")
                sendBroadcast(Intent(ACTION_REBOOT_REQUEST))
            }
            if (setSimpleDateFormatHour(System.currentTimeMillis()) == "11:59" && ScreenManager.currentScreen == Screen.STAND_BY) {
                Log.d(TAG, "chuanc mrn ${setSimpleDateFormatHour(System.currentTimeMillis())}")
                sendBroadcast(Intent(ACTION_REBOOT_REQUEST))
            }
            _binding.tvTime.text = setSimpleDateFormatHour(System.currentTimeMillis())
        }.also {
            runnableTimer = it
        }, timeDelay.toLong())
    }

    override fun finish() {
        super.finish()
        overridePendingTransition(R.anim.left_in, R.anim.right_out)
    }

    private val runnable = Runnable {
        viewPager2.currentItem = viewPager2.currentItem + 1
    }

    private fun init(){
        viewPager2 = findViewById(R.id.viewpager)
        handler = Handler(Looper.myLooper()!!)
        imageList = ArrayList()

        imageList.add(BannerModel( image = R.drawable.img_banner_default_three))
        imageList.add(BannerModel( image = R.drawable.img_banner_default_one))

        adapter = BannerAdapter(imageList, viewPager2)
        adapter.setOnClickItemBanner(object : BannerAdapter.OnClickItemBanner{
            override fun onClickItem(position: Int) {
                if (isValidBss()) {
                    LoginQRActivity.startActivity(this@ScreenStandbyActivity)
                }
            }
        })
        viewPager2.adapter = adapter
        viewPager2.clipToPadding = false
        viewPager2.clipChildren = false
        viewPager2.getChildAt(0).overScrollMode = RecyclerView.OVER_SCROLL_NEVER
    }

    private fun handleNumberPin(readPack: Int) {
        readyPack = readPack
        handleStatus()
    }
    @SuppressLint("ClickableViewAccessibility")
    private fun showNetworkDebug() {
        var timer = Timer()
        _binding.tvTime.setOnTouchListener(View.OnTouchListener { arg0, arg1 ->
            when (arg1.action) {
                MotionEvent.ACTION_DOWN -> {
                    //start timer
                    timer = Timer()
                    timer.schedule(object : TimerTask() {
                        override fun run() {
                            // invoke intent
                            startActivity(Chucker.getLaunchIntent(this@ScreenStandbyActivity))
                        }
                    }, 5000) //time out 5s
                    return@OnTouchListener true
                }
                MotionEvent.ACTION_UP -> {
                    //stop timer
                    timer.cancel()
                    return@OnTouchListener true
                }
            }
            false
        })
    }

    private fun handleStatus() {
        if (hasNetwork) {
            if (bssStatus?.status == Constants.ACTIVE ) {
                if (readyPack > 0) {
                    _binding.layoutMaintenance.visibility = View.GONE
                    _binding.tvTapOnScreen.visibility = View.VISIBLE
                    _binding.tvPinAvailble.text = readyPack.toString()
                    _binding.imageView4.visibility = View.VISIBLE
                } else {
                    _binding.tvPinAvailble.text = ""
                    _binding.layoutMaintenance.visibility = View.VISIBLE
                    _binding.tvStationMaintenance.text = resources.getString(R.string.station_out_of_battery)
                    _binding.icStatus.setImageResource(R.drawable.ic_no_battery)
                    _binding.tvTapOnScreen.visibility = View.GONE
                    _binding.imageView4.visibility = View.GONE
                }
            } else if (bssStatus == null) {
                _binding.tvPinAvailble.text = ""
                _binding.layoutMaintenance.visibility = View.VISIBLE
                _binding.tvStationMaintenance.text = "Không có dữ liệu"
                _binding.tvTapOnScreen.visibility = View.GONE
                _binding.icStatus.setImageResource(R.drawable.ic_no_internet)
                _binding.imageView4.visibility = View.GONE
            } else {
                _binding.tvPinAvailble.text = ""
                _binding.layoutMaintenance.visibility = View.VISIBLE
                _binding.tvStationMaintenance.text = resources.getString(R.string.station_maintenance)
                _binding.icStatus.setImageResource(R.drawable.ic_maintaince)
                _binding.tvTapOnScreen.visibility = View.GONE
                _binding.imageView4.visibility = View.GONE
            }
        } else {
            _binding.tvPinAvailble.text = ""
            _binding.layoutMaintenance.visibility = View.VISIBLE
            _binding.tvStationMaintenance.text = resources.getString(R.string.no_internet)
            _binding.tvTapOnScreen.visibility = View.GONE
            _binding.icStatus.setImageResource(R.drawable.ic_no_internet)
            _binding.imageView4.visibility = View.GONE
        }

    }

    private fun showLayoutMaintain() {
        _binding.apply {
            tvPinAvailble.text = ""
            layoutMaintenance.visibility = View.VISIBLE
            tvStationMaintenance.text = resources.getString(R.string.station_update)
            tvComeBackLater.text = resources.getString(R.string.come_back_later)
            icStatus.setImageResource(R.drawable.ic_update)
            tvTapOnScreen.visibility = View.GONE
            imageView4.visibility = View.GONE
        }
    }

    private fun isValidBss(): Boolean {
        return bssStatus?.status == Constants.ACTIVE && readyPack > 0 && hasNetwork && !App.isUpdatingApp
    }

    private fun handleBatteryStationStatus(data: BssStatusModel) {
        bssStatus = data;
        handleStatus()
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    override fun onNetworkChangeEvent(event: NetworkChangeEvent) {
        Log.d(TAG, "${javaClass.simpleName}: Internet change: isConnected = ${event.isConnected}")
        if (event.isConnected) {
         //   if (bssStatus == null) {
                viewModel.getBssStatus()
           // }
        }
        hasNetwork = event.isConnected
        handleStatus()
    }
}
