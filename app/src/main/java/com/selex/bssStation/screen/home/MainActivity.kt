package com.selex.bssStation.screen.home

import android.annotation.SuppressLint
import android.app.Activity
import android.app.AlarmManager
import android.app.AlertDialog
import android.app.PendingIntent
import android.content.DialogInterface
import android.content.Intent
import android.os.Bundle
import android.util.Log
import android.view.View
import android.view.View.OnClickListener
import android.view.WindowManager
import android.view.inputmethod.InputMethodManager
import androidx.activity.viewModels
import androidx.databinding.DataBindingUtil
import com.bumptech.glide.Glide
import com.bumptech.glide.load.engine.DiskCacheStrategy
import com.selex.bssStation.BuildConfig
import com.selex.bssStation.R
import com.selex.bssStation.core.utility.BottomDialogFragment
import com.selex.bssStation.core.utility.clickWithThrottle
import com.selex.bssStation.core.utility.showSnackbar
import com.selex.bssStation.core.utils.TAG
import com.selex.bssStation.core.utils.logout
import com.selex.bssStation.core.utils.properties.SystemProperties
import com.selex.bssStation.databinding.ActivityMainBinding
import com.selex.bssStation.model.user.UserModel
import com.selex.bssStation.model.user.isGuarantor
import com.selex.bssStation.screen.Screen
import com.selex.bssStation.screen.ScreenManager
import com.selex.bssStation.screen.base.BaseActivity
import com.selex.bssStation.screen.changeBattery.emptyCompartment.EmptyCompartmentActivity
import com.selex.bssStation.screen.syncData.SyncDataActivity
import com.selex.bssStation.service.BackgroundService
import com.selex.bssStation.service.NoActionCommon
import com.selex.bssStation.service.getSerialNumberBssFromJNI
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlin.system.exitProcess


@AndroidEntryPoint
class MainActivity : BaseActivity() {

    companion object {
        fun startActivity(activity: Activity, bundle: Bundle? = null,  model: UserModel) {
            val intent = Intent(activity, MainActivity::class.java)
            if (bundle != null) {
                intent.putExtras(bundle)
            }
            intent.putExtra("USER_MODEL",model)
            activity.startActivity(intent)
        }
    }

    private lateinit var binding: ActivityMainBinding
    private val viewModel: MainViewModel by viewModels()
    private var userModel: UserModel? = null
    private var isGotoEmptyScreen = false
    private var count = 0
    @SuppressLint("SetTextI18n")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = DataBindingUtil.setContentView(this, R.layout.activity_main)
        NoActionCommon.instance.startNoAction()
        setContentView(binding.root)

        if(intent.hasExtra("USER_MODEL")){
            val  model = intent.getSerializableExtra("USER_MODEL") as? UserModel
            if (model != null) {
                configData(model)
                binding.apply {
                    val serialNumber = getSerialNumberBssFromJNI()
                    tvSerial.text = "Số seri của trạm: $serialNumber"
                    edtSerial.setText(serialNumber)
                    tvSerial.visibility = if(model.isGuarantor()) View.VISIBLE else View.GONE
                    btnEnter.visibility = if(model.isGuarantor()) View.VISIBLE else View.GONE
                    btnExit.visibility = if(model.isGuarantor()) View.VISIBLE else View.GONE
                    edtSerial.visibility = if(model.isGuarantor()) View.VISIBLE else View.GONE
                    btnWriteSerial.visibility = if(model.isGuarantor()) View.VISIBLE else View.GONE
                }
                if(model.isGuarantor()) {
                    NoActionCommon.instance.cancelTimer()
                }
                userModel = model
            }
        }
        binding.btnStartChangeBattery.clickWithThrottle(throttleTime = 2000L){
            count++
            Log.d(TAG, "${javaClass.simpleName}: Start change battery----------------------------- $count $ScreenManager.currentScreen")
            if (!isGotoEmptyScreen && ScreenManager.currentScreen == Screen.MAIN) {
                NoActionCommon.instance.cancelTimer()
                if (binding.progressBarStartChange.visibility != View.VISIBLE) {
                    viewModel.getEmptyCabinet()
                }
                binding.progressBarStartChange.visibility = View.VISIBLE
            }
        }

        binding.btnLogout.setOnClickListener {
            logout(this)
        }
        binding.btnEnter.setOnClickListener {
            enterKioskMode()
        }
        binding.btnExit.setOnClickListener {
            exitKioskMode()
        }
        binding.btnWriteSerial.setOnClickListener {
            BackgroundService.isNeedSyncData = false
            val view = this.currentFocus
            if (view != null) {
                val imm = getSystemService(INPUT_METHOD_SERVICE) as InputMethodManager
                imm.hideSoftInputFromWindow(view.windowToken, 0)
            }
            SystemProperties.write(
                propName = SystemProperties.SERIAL_NUMBER,
                value = binding.edtSerial.text.toString(),
                onWriteSuccess = {
                    val alertDialog = AlertDialog.Builder(this)
                    alertDialog.apply {
                        setTitle("Ghi số seri thành công")
                        setMessage("Vui lòng khởi động lại ứng dụng để áp dụng số serial mới")
                        setPositiveButton(R.string.dialog_accept) { _, _ ->
                            val pendingIntent = PendingIntent.getActivity(
                                applicationContext,
                                0,
                                Intent(applicationContext, SyncDataActivity::class.java),
                                PendingIntent.FLAG_MUTABLE
                            )
                            val alarmManager = getSystemService(AlarmManager::class.java)
                            alarmManager.set(AlarmManager.RTC_WAKEUP, System.currentTimeMillis() + 100, pendingIntent)
                            finishAffinity()
                        }
                        setNegativeButton(R.string.dialog_cancel) { dialog, _ ->
                            dialog.dismiss()
                        }
                    }
                    val dialog = alertDialog.create()
                    dialog.show()
                    dialog.window!!.decorView.systemUiVisibility = (
                            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                                    or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION)
                    dialog.window!!.clearFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE)
                }
            )
        }

        viewModel.responseEmptyCabinet.observe(this) {
            binding.progressBarStartChange.visibility = View.GONE

           if (ScreenManager.currentScreen == Screen.MAIN) {
               if (it.data != null) {
                   Log.d(TAG, "${javaClass.simpleName}: GOTO di doi pin")
                   BackgroundService.isNeedSyncData = false
                   isGotoEmptyScreen = true
                   it.data.availableCabinetId?.let { it1 -> it.data.transactionId?.let { it2 ->
                       EmptyCompartmentActivity.startActivity(this, availableCabinetId = it1, transactionId = it2
                       )
                   } }
               } else {
                   NoActionCommon.instance.startNoAction()
                   it.error?.message?.let { it1 -> binding.bottomDialogFragment.getFragment<BottomDialogFragment>().show(message = it1, title = "Có lỗi xảy ra") }
               }
           }

        }
        binding.tvAddressStation.text = "Trạm " + BackgroundService.bss?.stationName
        binding.tvVersion.text = "v${BuildConfig.VERSION_NAME} b${BuildConfig.VERSION_CODE}"
    }
    @SuppressLint("SetTextI18n")
    private fun configData(model:UserModel) {
        binding.tvName.text = "Xin chào ${model.account?.firstName} ${model.account?.lastName}"
        Glide.with(this).load(model.account?.imageUrl).diskCacheStrategy(DiskCacheStrategy.ALL).placeholder(R.drawable.avtar_defaul).error(R.drawable.img_banner).into(binding.imgIconAvatar)

        if (model.subscription?.status == "EXPIRED") {
            binding.btnStartChangeBattery.visibility = View.GONE
            binding.layoutMaintenance.visibility = View.VISIBLE
            binding.icError.setImageResource(R.drawable.ic_nosubscription)
            binding.tvStationMaintenance.text = resources.getString(R.string.error_sub_expried)
            binding.tvComeBackLater.text = resources.getString(R.string.error_sub_expried_des)
        } else if (model.subscription?.status == "NO_SUB") {
            binding.tvStationMaintenance.text = resources.getString(R.string.error_no_sub)
            binding.tvComeBackLater.text = resources.getString(R.string.error_no_sub_des)
            binding.layoutMaintenance.visibility = View.VISIBLE
            binding.btnStartChangeBattery.visibility = View.GONE
            binding.icError.setImageResource(R.drawable.ic_nosubscription)
        } else {
            binding.btnStartChangeBattery.visibility = View.VISIBLE
            binding.layoutMaintenance.visibility = View.GONE
        }
    }

    override fun onResume() {
        super.onResume()
        Log.d(TAG, "MainActivity onResume")
        ScreenManager.currentScreen = Screen.MAIN
        isGotoEmptyScreen = false
        if (sharedPreferences.getToken().isEmpty() && userModel?.isGuarantor() == false) {
            logout(this)
        }
    }

    override fun onPause() {
        Log.i(TAG, "MainActivity: OnPause")
        super.onPause()
    }

    override fun onDestroy() {
        super.onDestroy()
        Log.i(TAG, "MainActivity: OnDestroy")
    }


    override fun finish() {
        super.finish()
        Log.i(TAG, "MainActivity: finish")
        overridePendingTransition(R.anim.left_in, R.anim.right_out)
    }

}