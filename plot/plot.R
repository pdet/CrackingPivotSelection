#devtools::install_github("slowkow/ggrepel")

(function(lp) {
np <- lp[!(lp %in% installed.packages()[,"Package"])]
if(length(np)) install.packages(np,repos=c("http://cran.rstudio.com/"))
x <- lapply(lp,function(x){library(x,character.only=TRUE, quietly=TRUE, warn.conflicts=FALSE)}) 
})(c("dplyr", "ggplot2", "ggthemes", "ggrepel", "data.table", "RColorBrewer"))


theme <- theme_few(base_size = 24)  +  theme(legend.position = "none", plot.title = element_text(size=16, hjust=.5, family="mono"))

dd <- fread("output.csv")

dd %>%  group_by(algorithm, query_pattern, repetition) %>%  mutate(query = 1:n(), cumtime=cumsum(query_time)) %>% as.data.frame() -> ddq

ddq %>% select(algorithm, query, query_pattern, query_time, cumtime) %>% group_by(algorithm, query, query_pattern) %>% summarise_all(funs(median)) -> pdata


ORDERING = c("pw"=1,"prwp"=2, 'prmwp'=3, 'pmwp'=4, 'pr'=5, 'prm'=6, 'pm'=7)
cols <- c("Workload"='#bdd09f',"Random Within Piece"='#493829',"Random Median Within Piece"='#8f3b1b',"Median Within Piece"='#4e6172',"Random"='#d57500',"Random Median"='#404f24',"Median "='#dbca69')


pdata %>% mutate(label=recode(algorithm, pw="Workload", prwp="Random Within Piece", prmwp="Random Median Within Piece", pmwp="Median Within Piece", pr="Random", prm="Random Median", pm="Median")) -> pdata2
pdata3 <- filter (pdata2, query < 50 )


pdata4<- filter (pdata3, query == 49)
pdf("performance_time.pdf", width=8, height=5)
ggplot(pdata3, aes(y=query_time, x=query, color=reorder(label, ORDERING[algorithm]), group=reorder(label, ORDERING[algorithm]), shape=reorder(label, ORDERING[algorithm]))) + geom_line( size=1)+ geom_point(data=pdata4,size=3)  + theme + ylab("Query Time (log(s))") + xlab("Query (#)") + theme (legend.position = c(0,0)) +  scale_y_log10() + guides(fill=guide_legend(nrow=3)) +theme(legend.justification = c(1, 1), legend.position = c(1, 1), legend.background = element_rect(fill = "transparent", colour = "transparent"), legend.title=element_blank()) + scale_color_manual(values=cols)
dev.off()

pdf("performance_time_cumulative.pdf", width=8, height=5)
ggplot(pdata3, aes(y=cumtime, x=query, color=reorder(label, ORDERING[algorithm]), group=reorder(label, ORDERING[algorithm]), shape=reorder(label, ORDERING[algorithm]))) + geom_line(size=1) + geom_point(data=pdata4,size=3) + theme + ylab("Cumulative Time (log(s))") + xlab("Query (#)") +scale_y_log10() + theme(legend.justification = c(0, 1), legend.position = c(0.65, 0.5), legend.background = element_rect(fill = "transparent", colour = "transparent"), legend.title=element_blank()) + scale_color_manual(values=cols)

dev.off()
