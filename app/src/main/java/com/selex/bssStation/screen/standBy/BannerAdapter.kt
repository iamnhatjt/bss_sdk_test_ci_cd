package com.selex.bssStation.screen.standBy

import android.annotation.SuppressLint
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import androidx.recyclerview.widget.RecyclerView
import androidx.viewpager2.widget.ViewPager2
import com.bumptech.glide.Glide
import com.bumptech.glide.load.engine.DiskCacheStrategy
import com.selex.bssStation.R
import com.selex.bssStation.model.banner.BannerModel

class BannerAdapter(val imageList: ArrayList<BannerModel>, private val viewPager2: ViewPager2) :
    RecyclerView.Adapter<BannerAdapter.BannerViewHolder>() {
    private lateinit var onClick: OnClickItemBanner

    interface OnClickItemBanner {
        fun onClickItem (position: Int)
    }

    fun setOnClickItemBanner(onClickItemBanner: OnClickItemBanner) {
        onClick = onClickItemBanner
    }
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): BannerViewHolder {
        val view = LayoutInflater.from(parent.context).inflate(R.layout.item_banner, parent, false)
        return BannerViewHolder(view)
    }
    override fun onBindViewHolder(holder: BannerViewHolder, position: Int) {
        holder.bind(imageList[position], onClick)
        if (position == imageList.size-1){
            viewPager2.post(runnable)
        }
    }

    override fun getItemCount(): Int {
        return imageList.size
    }

    private val runnable = Runnable {
        imageList.addAll(imageList)
        notifyDataSetChanged()
    }

    class BannerViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        private val imgBanner: ImageView = itemView.findViewById(R.id.img_item_banner)
        fun bind(data: BannerModel, onClickItemBanner: OnClickItemBanner) {
            Glide.with(itemView).load(data.image).diskCacheStrategy(DiskCacheStrategy.ALL).placeholder(R.drawable.img_banner).error(R.drawable.img_banner).into(imgBanner)

            itemView.setOnClickListener {
                onClickItemBanner.onClickItem(adapterPosition)
            }
        }
    }

    @SuppressLint("NotifyDataSetChanged")
    fun updateListBanner(list: List<BannerModel>) {
        imageList.clear()
        imageList.addAll(list)
        notifyDataSetChanged()
    }
}