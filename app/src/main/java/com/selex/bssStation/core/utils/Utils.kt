import android.annotation.SuppressLint
import okhttp3.MultipartBody
import okhttp3.RequestBody.Companion.asRequestBody
import java.io.File
import java.text.SimpleDateFormat
import java.util.*


@SuppressLint("SimpleDateFormat")
fun setSimpleDateFormatHour(time: Long): String {
    val simpleDate = SimpleDateFormat("HH:mm");
    simpleDate.timeZone = TimeZone.getTimeZone("Asia/Ho_Chi_Minh");
    return simpleDate.format(time)
}

@SuppressLint("SimpleDateFormat")
fun formatFullTime(time: Long): String {
    val simpleDate = SimpleDateFormat("HH_mm_dd_MM_yyyy");
    simpleDate.timeZone = TimeZone.getTimeZone("Asia/Ho_Chi_Minh");
    return simpleDate.format(time)
}


fun setIndexCabinetToBE(index: Int): Int {
    if (index < 0 || index > 19) {
        return index
    }
    return if (index <= 15) {
        index + 4
    } else {
        index - 15
    }
}

fun getIndexFromBE(index: Int): Int {
  //  return index
    return if (index >= 4) {
        index - 4
    } else {
        index + 15
    }
}

fun File.toMultipartFile() : MultipartBody.Part {
    return MultipartBody.Part.createFormData("file", this.name, this.asRequestBody())
}