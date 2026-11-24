<script setup lang="ts">
import {ref,onMounted,onBeforeUnmount,watch} from 'vue'
import {init,use,type ECharts} from 'echarts/core'
import {LineChart} from 'echarts/charts'
import {GridComponent} from 'echarts/components'
import {CanvasRenderer} from 'echarts/renderers'
use([LineChart,GridComponent,CanvasRenderer]);const props=defineProps<{samples:{ts_ms:number;value:number}[];end:number;windowMs:number}>();const host=ref<HTMLElement>();let chart:ECharts|undefined,ro:ResizeObserver|undefined
function draw(){chart?.setOption({animation:false,grid:{left:38,right:8,top:15,bottom:20},xAxis:{type:'value',min:-props.windowMs/1000,max:0,splitNumber:3,axisLabel:{fontSize:9,formatter:'{value}s'}},yAxis:{type:'value',scale:true,splitNumber:3,axisLabel:{fontSize:9}},series:[{type:'line',showSymbol:false,lineStyle:{width:2,color:'#258277'},data:props.samples.map(p=>[(p.ts_ms-props.end)/1000,p.value])}]})}
onMounted(()=>{if(!host.value)return;ro=new ResizeObserver(()=>{if(!host.value?.clientWidth)return;if(!chart)chart=init(host.value);chart.resize();draw()});ro.observe(host.value)});watch(()=>props.end,draw);onBeforeUnmount(()=>{ro?.disconnect();chart?.dispose()})
</script><template><div ref="host" class="plot" aria-label="测量值时间曲线"/></template><style scoped>.plot{height:160px;width:100%;background:#fff;border:1px solid #d6e0db;border-radius:4px}</style>